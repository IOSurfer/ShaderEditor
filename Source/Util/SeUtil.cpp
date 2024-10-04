#include "SeUtil.h"
#include <QDebug>
#include <fstream>
#include <iostream>

std::vector<char> SeUtil::readFile(const std::string &file_name) {
    std::ifstream file(file_name, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        qDebug() << "Fail to open file: " << file_name;
    }

    size_t file_size = (size_t)file.tellg();
    std::vector<char> buffer(file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();

    return buffer;
}