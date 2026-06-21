#include "SystemCatalog.h"
#include <fstream>
#include <filesystem>
#include <cstring>

using namespace std;

namespace fs = filesystem;

SystemCatalog::SystemCatalog() {
    catalogPath = "SystemCatalog";
    ensureCatalogFolder();
}

SystemCatalog::~SystemCatalog() {
}

void SystemCatalog::ensureCatalogFolder() {
    if (!fs::exists(catalogPath)) {
        fs::create_directory(catalogPath);
    }
}

void SystemCatalog::copyString(char* dest, const string& src, size_t maxLen) {
    size_t len = src.length();
    if (len >= maxLen) len = maxLen - 1;
    for (size_t i = 0; i < len; ++i) {
        dest[i] = src[i];
    }
    dest[len] = '\0';
}

bool SystemCatalog::addDatabase(const string& dbName) {
    string filePath = catalogPath + "/SystemDatabases.bin";
    ofstream file(filePath, ios::binary | ios::app);
    if (!file.is_open()) return false;

    DBCatalogRecord rec;
    copyString(rec.name, dbName, 64);

    file.write(reinterpret_cast<const char*>(&rec), sizeof(DBCatalogRecord));
    file.close();
    return true;
}

bool SystemCatalog::addTable(const string& dbName, const string& tableName) {
    string filePath = catalogPath + "/SystemTables.bin";
    ofstream file(filePath, ios::binary | ios::app);
    if (!file.is_open()) return false;

    TableCatalogRecord rec;
    copyString(rec.dbName, dbName, 64);
    copyString(rec.tableName, tableName, 64);

    file.write(reinterpret_cast<const char*>(&rec), sizeof(TableCatalogRecord));
    file.close();
    return true;
}

bool SystemCatalog::addColumn(const string& dbName, const string& tableName, const string& colName, const string& type, size_t length) {
    string filePath = catalogPath + "/SystemColumns.bin";
    ofstream file(filePath, ios::binary | ios::app);
    if (!file.is_open()) return false;

    ColumnCatalogRecord rec;
    copyString(rec.dbName, dbName, 64);
    copyString(rec.tableName, tableName, 64);
    copyString(rec.columnName, colName, 64);
    copyString(rec.type, type, 20);
    rec.length = length;

    file.write(reinterpret_cast<const char*>(&rec), sizeof(ColumnCatalogRecord));
    file.close();
    return true;
}

bool SystemCatalog::addIndex(const string& dbName, const string& tableName, const string& colName, const string& indexName, const string& indexType) {
    string filePath = catalogPath + "/SystemIndexes.bin";
    ofstream file(filePath, ios::binary | ios::app);
    if (!file.is_open()) return false;

    IndexCatalogRecord rec;
    copyString(rec.dbName, dbName, 64);
    copyString(rec.tableName, tableName, 64);
    copyString(rec.columnName, colName, 64);
    copyString(rec.indexName, indexName, 64);
    copyString(rec.indexType, indexType, 20);

    file.write(reinterpret_cast<const char*>(&rec), sizeof(IndexCatalogRecord));
    file.close();
    return true;
}

vector<string> SystemCatalog::getDatabases() {
    vector<string> dbs;
    string filePath = catalogPath + "/SystemDatabases.bin";
    ifstream file(filePath, ios::binary);
    if (!file.is_open()) return dbs;

    DBCatalogRecord rec;
    while (file.read(reinterpret_cast<char*>(&rec), sizeof(DBCatalogRecord))) {
        dbs.push_back(string(rec.name));
    }
    file.close();
    return dbs;
}

vector<string> SystemCatalog::getTables(const string& dbName) {
    vector<string> tables;
    string filePath = catalogPath + "/SystemTables.bin";
    ifstream file(filePath, ios::binary);
    if (!file.is_open()) return tables;

    TableCatalogRecord rec;
    while (file.read(reinterpret_cast<char*>(&rec), sizeof(TableCatalogRecord))) {
        if (string(rec.dbName) == dbName) {
            tables.push_back(string(rec.tableName));
        }
    }
    file.close();
    return tables;
}

vector<ColumnCatalogRecord> SystemCatalog::getColumns(const string& dbName, const string& tableName) {
    vector<ColumnCatalogRecord> columns;
    string filePath = catalogPath + "/SystemColumns.bin";
    ifstream file(filePath, ios::binary);
    if (!file.is_open()) return columns;

    ColumnCatalogRecord rec;
    while (file.read(reinterpret_cast<char*>(&rec), sizeof(ColumnCatalogRecord))) {
        if (string(rec.dbName) == dbName && string(rec.tableName) == tableName) {
            columns.push_back(rec);
        }
    }
    file.close();
    return columns;
}

vector<IndexCatalogRecord> SystemCatalog::getIndexes() {
    vector<IndexCatalogRecord> indexes;
    string filePath = catalogPath + "/SystemIndexes.bin";
    ifstream file(filePath, ios::binary);
    if (!file.is_open()) return indexes;

    IndexCatalogRecord rec;
    while (file.read(reinterpret_cast<char*>(&rec), sizeof(IndexCatalogRecord))) {
        indexes.push_back(rec);
    }
    file.close();
    return indexes;
}

vector<IndexCatalogRecord> SystemCatalog::getTableIndexes(const string& dbName, const string& tableName) {
    vector<IndexCatalogRecord> indexes;
    string filePath = catalogPath + "/SystemIndexes.bin";
    ifstream file(filePath, ios::binary);
    if (!file.is_open()) return indexes;

    IndexCatalogRecord rec;
    while (file.read(reinterpret_cast<char*>(&rec), sizeof(IndexCatalogRecord))) {
        if (string(rec.dbName) == dbName && string(rec.tableName) == tableName) {
            indexes.push_back(rec);
        }
    }
    file.close();
    return indexes;
}