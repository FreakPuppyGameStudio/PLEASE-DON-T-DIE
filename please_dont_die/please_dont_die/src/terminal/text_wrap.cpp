#include "terminal/text_wrap.h"

#include <sstream>

std::vector<std::string> wrapText(const std::string& text, float maxWidth,
                                  const std::function<float(const std::string&)>& measure) {
    std::vector<std::string> lines;
    std::string current;
    std::istringstream words(text);
    std::string word;
    while (words >> word) {
        const std::string candidate = current.empty() ? word : current + " " + word;
        if (!current.empty() && measure(candidate) > maxWidth) {
            lines.push_back(current);
            current = word;
        } else {
            current = candidate;
        }
    }
    lines.push_back(current);
    return lines;
}
