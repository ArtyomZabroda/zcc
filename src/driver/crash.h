#ifndef DRIVER_CRASH_H_
#define DRIVER_CRASH_H_

#include <format>
#include <iostream>

namespace zcc::driver {

template<typename... Args>
void Crash(std::format_string<Args...> fmt, Args&&... args) {
  std::cerr << std::format("error: " + fmt, std::forward<Args>(args)...);
  exit(EXIT_FAILURE);
}

}


#endif