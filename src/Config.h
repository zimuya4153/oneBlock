#pragma once

#include <string>
#include <vector>

namespace oneBlock {
struct Config {
    int                      version   = 1; // 配置文件版本
    std::vector<std::string> blacklist = {"minecraft:unknown", "minecraft:border_block"};
};
} // namespace oneBlock