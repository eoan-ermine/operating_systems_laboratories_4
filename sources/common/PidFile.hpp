#pragma once

#include <filesystem>
#include <string>

class PidFile {
public:
  PidFile(const std::string_view &path, const std::string_view &name);
  ~PidFile();

  bool tryLock();

  bool hasLock() const;
  bool setPID(uint32_t);
  uint32_t getPID() const;

  std::string getPath() const { return path.string(); }
  int getDescriptor() const { return descriptor; }

  static PidFile tmp(const std::string_view &name);
  static PidFile var(const std::string_view &name);

private:
  std::filesystem::path path;
  int descriptor = -1;
};
