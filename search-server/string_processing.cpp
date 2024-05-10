#include "string_processing.h"
#include <sstream>

std::vector<std::string> SplitIntoWords(const std::string& text) {
    std::vector<std::string> words;
    std::string word;
    std::istringstream is(text);
    while (is >> word) {
        words.push_back(word);
    }
    return words;
}
