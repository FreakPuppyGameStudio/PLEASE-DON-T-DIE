#pragma once
// Word wrap that does not depend on Raylib: the caller supplies the measuring function.

#include <functional>
#include <string>
#include <vector>

std::vector<std::string> wrapText(const std::string& text, float maxWidth,
                                  const std::function<float(const std::string&)>& measure);
