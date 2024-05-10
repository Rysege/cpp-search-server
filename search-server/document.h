#pragma once
#include <iostream>

struct Document {
    Document() = default;
    Document(int _id, double _relevance, int _rating)
        : id(_id)
        , relevance(_relevance)
        , rating(_rating) {
    }
    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

std::ostream& operator<<(std::ostream& out, const Document& document);