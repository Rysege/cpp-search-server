# pragma once
#include <algorithm>
#include <iostream>
#include <vector>

template <typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator begin, Iterator end)
        : begin_(begin)
        , end_(end)
        , size_(distance(begin, end)) {
    }

    auto begin() const {
        return begin_;
    }
    auto end()const {
        return end_;
    }
    size_t size() const {
        return size_; 
    }

private:
    Iterator begin_, end_;
    size_t size_;
};

template <typename Iterator>
std::ostream& operator<<(std::ostream& out, const IteratorRange<Iterator>& range) {
    for (auto it = range.begin(); it != range.end(); ++it) {
        out << *it;
    }
    return out;
}

template <typename Iterator>
class Paginator {
public:
    Paginator(Iterator range_begin, Iterator range_end, size_t page_size) {
        while (range_begin != range_end && page_size) {
            page_size = std::min<size_t>(std::distance(range_begin, range_end), page_size);
            const auto current_page_end = std::next(range_begin, page_size);
            pages_.push_back({ range_begin, current_page_end });
            range_begin = current_page_end;
        }
    }

    auto begin() const {
        return pages_.begin();
    }
    auto end() const { 
        return pages_.end();
    }
    size_t size() const {
        return pages_.size(); 
    }

private:
    std::vector<IteratorRange<Iterator>> pages_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}