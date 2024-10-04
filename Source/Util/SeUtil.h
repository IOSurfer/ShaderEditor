#ifndef SE_UTIL_H
#define SE_UTIL_H

#include <string>
#include <vector>

class SeUtil {
  public:
    static std::vector<char> readFile(const std::string &file_name);
};

#endif