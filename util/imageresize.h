#pragma once

#include <stdint.h>
#include <vector>

namespace ImageUtil{

void limitImageSize(const std::vector<uint8_t>& in, std::vector<uint8_t>& out, uint32_t width, uint32_t height);

} // namespace ImageUtil
