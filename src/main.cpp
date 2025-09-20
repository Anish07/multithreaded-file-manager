
#include <iostream>
#include <filesystem>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <atomic>
#include <optional>

namespace fs = std::filesystem;

struct Stats {
    std::atomic<uint64_t> files{0};
    std::atomic<uint64_t> dirs{0};
    std::atomic<uint64_t> bytes{0};
};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: multithreaded_file_manager <root_path> [threads]\n";
        return 1;
    }
    fs::path root = argv[1];
    size_t threads = (argc >= 3)? std::stoul(argv[2]) : std::thread::hardware_concurrency();
    if (threads == 0) threads = 1;

    std::queue<fs::path> q;
    std::mutex m;
    std::condition_variable cv;
    bool done = false;
    Stats stats;

    // seed
    {
        std::lock_guard<std::mutex> lg(m);
        q.push(root);
    }
    cv.notify_all();

    auto worker = [&]{
        while (true) {
            fs::path p;
            {
                std::unique_lock<std::mutex> lk(m);
                cv.wait(lk, [&]{ return !q.empty() || done; });
                if (q.empty()) {
                    if (done) return;
                    else continue;
                }
                p = q.front();
                q.pop();
            }

            try {
                if (fs::is_directory(p)) {
                    stats.dirs++;
                    for (auto& entry : fs::directory_iterator(p)) {
                        std::lock_guard<std::mutex> lg(m);
                        q.push(entry.path());
                        cv.notify_one();
                    }
                } else if (fs::is_regular_file(p)) {
                    stats.files++;
                    std::error_code ec;
                    auto sz = fs::file_size(p, ec);
                    if (!ec) stats.bytes += sz;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << " at " << p << "\n";
            }
        }
    };

    std::vector<std::thread> pool;
    for (size_t i = 0; i < threads; ++i) pool.emplace_back(worker);

    // monitor queue; quit when exhausted
    while (true) {
        {
            std::unique_lock<std::mutex> lk(m);
            if (q.empty()) {
                done = true;
                cv.notify_all();
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    for (auto& t : pool) t.join();

    std::cout << "Dirs: " << stats.dirs.load() << "\n"
              << "Files: " << stats.files.load() << "\n"
              << "Bytes: " << stats.bytes.load() << "\n";
    return 0;
}
