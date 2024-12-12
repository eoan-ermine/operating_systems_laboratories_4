#include <atomic>
#include <charconv>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <iostream>
#include <random>
#include <string_view>
#include <thread>

#include "common/PidFile.hpp"
#include "common/globals.hpp"

using namespace std::string_view_literals;

std::atomic<bool> is_running{false};

int main(int argc, char *argv[]) {
  constexpr std::string_view USAGE = "Usage: leaker (off|on) [ALLOC_SIZE]";

  if (argc < 2) {
    std::cerr << USAGE << std::endl;
    return 1;
  }

  auto pid_file = PidFile::tmp(LEAKER_APPNAME);
  if (!pid_file.tryLock()) {
    std::cerr << "Can't lock " << pid_file.getPath() << std::endl;
    return 1;
  } else {
    pid_file.setPID(getpid());
  }

  std::signal(SIGINT, [](int signal) { is_running = false; });
  std::signal(SIGUSR1, [](int signal) { is_running = false; });

  double free_percent = (argv[1] == "off"sv) ? 100 : 75;
  std::size_t alloc_size = 1000;
  if (argc >= 3) {
    std::string_view alloc_size_arg = argv[2];
    auto [_, ec] = std::from_chars(
        alloc_size_arg.data(), alloc_size_arg.data() + alloc_size_arg.size(),
        alloc_size);
    if (ec != std::errc{}) {
      std::cerr << USAGE << std::endl;
      return 1;
    }
  }

  std::cout << "Entering allocation loop with ALLOC_SIZE = " << alloc_size
            << std::endl;

  std::vector<unsigned char *> memory_descriptors;
  std::mutex cv_mutex;
  std::condition_variable cv;

  std::random_device random_device;
  std::mt19937 random_engine(random_device());
  std::uniform_int_distribution sleep_distribution(100, 1000);

  is_running = true;

  std::thread deallocator([&, random_engine]() mutable {
    std::uniform_int_distribution free_distribution(0, 100);
    while (is_running) {
      std::unique_lock lk(cv_mutex);
      cv.wait(lk);

      if (memory_descriptors.size() > 0) {
        auto ptr = memory_descriptors.back();
        if (free_distribution(random_engine) <= free_percent) {
          delete[] ptr;
          memory_descriptors.pop_back();
        }
      }
    }

    for (auto ptr: memory_descriptors) {
      delete[] ptr;
    }
  });

  while (is_running) {
    unsigned char *ptr = new unsigned char[alloc_size];
    
    {
      std::lock_guard lk(cv_mutex);
      memory_descriptors.push_back(ptr);
    }
    cv.notify_one();

    std::this_thread::sleep_for(
        std::chrono::nanoseconds(sleep_distribution(random_engine)));
  }

  cv.notify_all();
  deallocator.join();
}