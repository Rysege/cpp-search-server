#include "document.h"

std::ostream& operator<<(std::ostream& out, const Document& document) {
    using namespace std::string_literals;
    return out << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s;
}