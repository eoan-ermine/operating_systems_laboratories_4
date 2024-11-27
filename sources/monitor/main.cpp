#include <cstring>
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

#include "common/PidFile.hpp"
#include "common/globals.hpp"

int main() {
  PidFile pid_file = PidFile::tmp(LEAKER_APPNAME);
  if (pid_file.getPID() == 0) {
    std::cerr << "Leaker is not running. Terminating" << std::endl;
    return 1;
  }

  int PID = fork();
  if (PID == 0) {
    std::string pid_string = std::to_string(pid_file.getPID());
    const char *argv[] = {"mapper", pid_string.c_str(),
                          MONITOR_DUMPS_PATH.data(), NULL};
    if (execv(MAPPER_PATH.data(), const_cast<char **>(argv)) != 0) {
      std::cerr << "Can't run mapper: " << errno << ": " << strerror(errno)
                << std::endl;
      return 1;
    }
  }

  int status;
  auto w = waitpid(PID, &status, 0);
  if (w == -1) {
    std::cerr << "waitpid(...) error: " << errno << ": " << strerror(errno)
              << std::endl;
    return 1;
  }
  if (WEXITSTATUS(status) != 0) {
    std::cerr << "mapper return code: " << WEXITSTATUS(status) << std::endl;
    return 1;
  }
}