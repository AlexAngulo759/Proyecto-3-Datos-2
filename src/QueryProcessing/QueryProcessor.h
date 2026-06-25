#ifndef QUERYPROCESSOR_H
#define QUERYPROCESSOR_H

#include <string>
#include <vector>
#include <chrono>
#include "SystemCatalog.h"
#include "../StoredDataManager/StorageManager.h"
#include "../StoredDataManager/IndexBST.h"
#include "../StoredDataManager/IndexBTree.h"
#include <map>

struct QueryResult {
    bool success;
    std::string message;
    std::vector<std::vector<std::string>> rows;
    double executionTime;
};

class QueryProcessor {
private:
    SystemCatalog catalog;
    StorageManager storage;
    std::string currentDatabase;
    

    std::string normalize(const std::string& query);
    std::vector<std::string> split(const std::string& str, char delimiter);

    QueryResult handleCreateDatabase(const std::string& query);
    QueryResult handleSetDatabase(const std::string& query);
    QueryResult handleCreateTable(const std::string& query);
    QueryResult handleInsert(const std::string& query);
    QueryResult handleSelect(const std::string& query);
    QueryResult handleUpdate(const std::string& query);
    QueryResult handleDelete(const std::string& query);
    QueryResult handleCreateIndex(const std::string& query);

    void quickSort(
        std::vector<std::vector<std::string>>& rows,
        int low,
        int high,
        int columnIndex,
        bool asc
    );

    int partition(
        std::vector<std::vector<std::string>>& rows,
        int low,
        int high,
        int columnIndex,
        bool asc
    );

    //sebas

    std::map<std::string, IndexBST> bstIndexes;
    std::map<std::string, IndexBTree> btreeIndexes;
    

public:
    QueryProcessor();

    QueryResult execute(const std::string& query);
};

#endif