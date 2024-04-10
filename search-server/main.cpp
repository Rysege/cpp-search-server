#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (iscntrl(c)) {
            throw invalid_argument("contains invalid characters"s);
        }
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}

struct Document {
    Document() = default;

    Document(int _id, double _relevance, int _rating)
        : id(_id), relevance(_relevance), rating(_rating) {
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

void CheckForExcessMinus(const string& word) {
    if (word == "-"s) {
        throw invalid_argument("minus without a word"s);
    }
    if (word.substr(0, 2) == "--") {
        throw invalid_argument("double minus"s);
    }
}

void CheckForCntrlChar(const string& word) {
    if (any_of(word.begin(), word.end(), [](const unsigned char c) { return iscntrl(c); })) {
        throw invalid_argument("contains invalid characters"s);
    }
}

template<typename StringContainer>
set<string> MakeUniqueNonEmptyStrings(StringContainer& container, bool check_cntrl = true) {
    set<string> non_empty_strings;
    for (const string& word : container) {
        if (check_cntrl) {
            CheckForCntrlChar(word);
        }
        if (!word.empty()) {
            non_empty_strings.insert(word);
        }
    }
    return non_empty_strings;
}

class SearchServer {
public:
    template<typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words, bool check_cntrl = true)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words, check_cntrl)) {}

    explicit SearchServer(const string& stop_words_text) 
        : SearchServer(SplitIntoWords(stop_words_text), false) {}

    int GetDocumentCount() const {
        return static_cast<int>(documents_.size());
    }

    int GetDocumentId(int index) const {
        if (index < 0 || index > GetDocumentCount()) {
            throw out_of_range("index of transmitted document is out of acceptable range"s);
        }
        auto it = next(documents_.begin(), index);
        return it->first;
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
        CheckCorrectID(document_id);
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    }

    template <typename FilterPredicate>
    vector<Document> FindTopDocuments(const string& raw_query, FilterPredicate filter_predicate) const {
        const Query& query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, filter_predicate);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                return lhs.relevance > rhs.relevance
                    || (abs(lhs.relevance - rhs.relevance) < EPSILON && lhs.rating > rhs.rating);
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus document_status = DocumentStatus::ACTUAL) const {
        return FindTopDocuments(raw_query, [document_status](int, DocumentStatus status, int) { return status == document_status; });
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const {
        const Query query = ParseQuery(raw_query);
        vector<string> words;

        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) && word_to_document_freqs_.at(word).count(document_id)) {
                words.push_back(word);
            }
        }

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) && word_to_document_freqs_.at(word).count(document_id)) {
                words.clear();
            }
        }
        return tuple<vector<string>, DocumentStatus>{ words, documents_.at(document_id).status };
    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;

    void CheckCorrectID(int id) const {
        if (id < 0) {
            throw invalid_argument("negative id"s);
        }
        if (documents_.count(id)) {
            throw invalid_argument("there is already such an id"s);
        }
    }

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = 0;
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        return { text, is_minus, IsStopWord(text) };
    }

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
            CheckForExcessMinus(word);
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                (query_word.is_minus ? query.minus_words : query.plus_words).insert(query_word.data);
            }
        }
        return query;
    }

    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(documents_.size() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    template <typename FilterPredicate>
    vector<Document> FindAllDocuments(const Query& query, FilterPredicate filter_predicate) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto& [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                if (filter_predicate(document_id, documents_.at(document_id).status, documents_.at(document_id).rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }
        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto& [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto& [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
        }
        return matched_documents;
    }
};

void PrintDocument(const Document& document) {
    cout << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating
        << " }"s << endl;
}

int main() {
    try {
        SearchServer search_server("and a with"s);
        search_server.AddDocument(0, "a white cat and a fashionable collar"s, DocumentStatus::ACTUAL, { 8, -3 });
        search_server.AddDocument(1, "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
        search_server.AddDocument(2, "a well-groomed dog with expressive eyes"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
        search_server.AddDocument(3, "well-groomed starling Eugene"s, DocumentStatus::BANNED, { 9 });
        cout << "ACTUAL by default:"s << endl;
        for (const Document& document : search_server.FindTopDocuments("fluffy well-groomed cat"s)) {
            PrintDocument(document);
        }
        cout << "BANNED:"s << endl;
        for (const Document& document : search_server.FindTopDocuments("fluffy well-groomed cat"s, DocumentStatus::BANNED)) {
            PrintDocument(document);
        }
        cout << "Even ids:"s << endl;
        for (const Document& document : search_server.FindTopDocuments("fluffy well-groomed cat"s, [](int document_id, DocumentStatus, int) { return document_id % 2 == 0; })) {
            PrintDocument(document);
        }
    }
    catch (const invalid_argument& e) {
        cout << "Error: "s << e.what() << endl;
    }
    catch (const out_of_range& e) {
        cout << "Error: "s << e.what() << endl;
    }
    catch (...) {
        cout << "Unexpected error"s << endl;
    }
}