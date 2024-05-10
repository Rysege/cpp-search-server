#include "paginator.h"
#include "request_queue.h"
#include <iostream>

using namespace std;

int main()
{
    try {
        SearchServer search_server("and a with"s);
        RequestQueue request_queue(search_server);
        search_server.AddDocument(0, "a white cat and a fashionable collar"s, DocumentStatus::ACTUAL, { 8, -3 });
        search_server.AddDocument(1, "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
        search_server.AddDocument(2, "a well-groomed dog with expressive eyes"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
        search_server.AddDocument(3, "well-groomed starling Eugene"s, DocumentStatus::BANNED, { 9 });
        search_server.AddDocument(4, "curly cat curly tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
        search_server.AddDocument(5, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
        search_server.AddDocument(6, "big cat fancy collar "s, DocumentStatus::ACTUAL, { 1, 2, 8 });
        search_server.AddDocument(7, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
        search_server.AddDocument(8, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, { 1, 1, 1 });

        cout << "ACTUAL by default:"s << endl;
        const auto search_results = search_server.FindTopDocuments("fluffy well-groomed cat"s);
        int page_size = 2;
        const auto pages = Paginate(search_results, page_size);
        for (auto page = pages.begin(); page != pages.end(); ++page) {
            cout << *page << endl;
            cout << "Page break"s << endl;
        }
        cout << "BANNED:"s << endl;
        for (const Document& document : search_server.FindTopDocuments("fluffy well-groomed cat"s, DocumentStatus::BANNED)) {
            cout << document << endl;
        }
        cout << "Even ids:"s << endl;
        for (const Document& document : search_server.FindTopDocuments("curly dog"s, [](int document_id, DocumentStatus, int) { return document_id % 2 == 0; })) {
            cout << document << endl;
        }
        // 1439 запросов с нулевым результатом
        for (int i = 0; i < 1439; ++i) {
            request_queue.AddFindRequest("empty request"s);
        }
        // все еще 1439 запросов с нулевым результатом
        request_queue.AddFindRequest("curly dog"s);
        // новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
        request_queue.AddFindRequest("big collar"s);
        // первый запрос удален, 1437 запросов с нулевым результатом
        request_queue.AddFindRequest("sparrow"s);
        cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;
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
    return 0;
}
