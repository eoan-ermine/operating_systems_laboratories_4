#include "PidFile.hpp"
#include <fcntl.h>
#include <unistd.h>

PidFile::PidFile(const std::string_view &path_, const std::string_view &name)
    : path(std::filesystem::path(path_) / (std::string{name} + ".pid")) {}

PidFile::~PidFile() {
  if (descriptor >= 0) {
    ::close(descriptor);
    std::filesystem::remove(path);
  }
}

bool PidFile::tryLock() {
  descriptor = ::open(path.string().c_str(), O_RDWR | O_CREAT | O_EXCL, 0644);
  if (descriptor >= 0) {
    struct flock lock_info = {};
    lock_info.l_type = F_WRLCK;    /* exclusive write lock */
    lock_info.l_whence = SEEK_SET; /* use start and len */
    if (::fcntl(descriptor, F_SETLK, &lock_info) < 0) {
      ::close(descriptor);
      descriptor = -1;
    }
  }
  return descriptor >= 0;
}

bool PidFile::hasLock() const { return descriptor >= 0; }

bool PidFile::setPID(uint32_t pid) {
  if (descriptor >= 0 && ftruncate(descriptor, 0) >= 0) {
    return dprintf(descriptor, "%u", pid) > 0;
  }
  return false;
}

uint32_t PidFile::getPID() const {
  if (auto file = fopen(path.string().c_str(), "r"); file) {
    if (char pid[16] = {}; fgets(pid, sizeof(pid), file)) {
      return std::stoul(pid);
    }
  }
  return 0;
}

PidFile PidFile::tmp(const std::string_view &name) {
  return PidFile("/tmp", name);
}

PidFile PidFile::var(const std::string_view &name) {
  return PidFile("/var/run", name);
}
