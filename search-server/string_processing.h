#pragma once
#include <set>
#include <string>
#include <vector>

std::vector<std::string> SplitIntoWords(const std::string& text);

template<typename StringContainer>
std::set<std::string> MakeUniqueNonEmptyStrings(StringContainer& container) {
    std::set<std::string> non_empty_strings;
    for (const std::string& word : container) {
        if (!word.empty()) {
            non_empty_strings.insert(word);
        }
    }
    return non_empty_strings;
}