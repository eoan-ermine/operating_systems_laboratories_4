#include <charconv>
#include <csignal>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string_view>

const std::string currentDateTime() {
  time_t now = time(0);
  struct tm tstruct;
  tstruct = *localtime(&now);

  char buf[80];
  strftime(buf, sizeof(buf), R"(%Y-%m-%d_%H:%M:%S)", &tstruct);
  return buf;
}

std::filesystem::path getMapPath(std::filesystem::path dir, std::size_t PID) {
  auto current_time = currentDateTime();
  return std::format("{}/map_{}_{}", dir.string(), PID, currentDateTime());
}

int main(int argc, char *argv[]) {
  constexpr std::string_view USAGE = "Usage: mapper PID SAVE_DIR_PATH";

  if (argc < 3) {
    std::cerr << USAGE << std::endl;
    return 1;
  }

  std::size_t PID;
  std::string_view PID_arg = argv[1];
  std::string_view save_dir_path_arg = argv[2];

  auto [_, ec] =
      std::from_chars(PID_arg.data(), PID_arg.data() + PID_arg.size(), PID);
  if (ec != std::errc{}) {
    std::cerr << USAGE << std::endl;
    return 1;
  }
  if (kill(PID, 0) != 0) {
    std::cout << "Process with PID " << PID << " doesn't exist" << std::endl;
    return 1;
  }

  auto map_path = getMapPath(save_dir_path_arg, PID);
  std::ofstream output_stream{getMapPath(save_dir_path_arg, PID),
                              std::ios::out};
  if (!output_stream) {
    std::cerr << "Can't open " << map_path << std::endl;
    return 1;
  }

  auto pmap_cmdline = std::format("pmap -x {}", PID);
  auto pipe = popen(pmap_cmdline.c_str(), "r");
  if (!pipe) {
    std::cerr << "Couldn't execute " << pmap_cmdline << std::endl;
    return 1;
  }

  std::array<char, 128> buffer;
  std::string result;
  while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
    result += buffer.data();
  }

  auto pmap_return_code = pclose(pipe);
  if (pmap_return_code != 0) {
    std::clog << "pmap return code: " << pmap_return_code << std::endl;
  }

  output_stream << result;
  return 0;
}