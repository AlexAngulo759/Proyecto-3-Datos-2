#ifndef SYSTEMCATALOG_H
#define SYSTEMCATALOG_H

#include <string>
#include <vector>

struct DBCatalogRecord {
    char name[64];
};

struct TableCatalogRecord {
    char dbName[64];
    char tableName[64];
};

struct ColumnCatalogRecord {
    char dbName[64];
    char tableName[64];
    char columnName[64];
    char type[20];
    size_t length;
};

struct IndexCatalogRecord {
    char dbName[64];
    char tableName[64];
    char columnName[64];
    char indexName[64];
    char indexType[20];
};

class SystemCatalog {
private:
    std::string catalogPath;

    void ensureCatalogFolder();
    void copyString(char* dest, const std::string& src, size_t maxLen);

public:
    SystemCatalog();
    ~SystemCatalog();

    bool addDatabase(const std::string& dbName);
    bool addTable(const std::string& dbName, const std::string& tableName);
    bool addColumn(const std::string& dbName, const std::string& tableName, const std::string& colName, const std::string& type, size_t length);
    bool addIndex(const std::string& dbName, const std::string& tableName, const std::string& colName, const std::string& indexName, const std::string& indexType);

    std::vector<std::string> getDatabases();
    std::vector<std::string> getTables(const std::string& dbName);
    std::vector<ColumnCatalogRecord> getColumns(const std::string& dbName, const std::string& tableName);
    std::vector<IndexCatalogRecord> getIndexes();
    std::vector<IndexCatalogRecord> getTableIndexes(const std::string& dbName, const std::string& tableName);
};

#endif