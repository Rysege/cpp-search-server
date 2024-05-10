#include "request_queue.h"

RequestQueue::RequestQueue(const SearchServer& search_server)
    : search_server_(search_server)
    , no_results_requests_(0)
    , current_time_(0) {
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    const auto matched_documents = search_server_.FindTopDocuments(raw_query, status);
    AddRequest(matched_documents.size());
    return matched_documents;
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
    const auto matched_documents = search_server_.FindTopDocuments(raw_query);
    AddRequest(matched_documents.size());
    return matched_documents;
}

int RequestQueue::GetNoResultRequests() const {
    return no_results_requests_;
}

void RequestQueue::AddRequest(size_t number_of_results) {
    ++current_time_;
    while (!requests_.empty() && min_in_day_ <= current_time_ - requests_.front().timestamp) {
        if (!requests_.front().results) {
            --no_results_requests_;
        }
        requests_.pop_front();
    }
    if (!number_of_results) {
        ++no_results_requests_;
    }
    requests_.push_back({ current_time_, number_of_results });
}
