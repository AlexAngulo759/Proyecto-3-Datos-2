#include "SystemCatalog.h"
#include <fstream>
#include <filesystem>
#include <cstring>

namespace fs = std::filesystem;

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

void SystemCatalog::copyString(char* dest, const std::string& src, size_t maxLen) {
    size_t len = src.length();
    if (len >= maxLen) len = maxLen - 1;
    for (size_t i = 0; i < len; ++i) {
        dest[i] = src[i];
    }
    dest[len] = '\0';
}

bool SystemCatalog::addDatabase(const std::string& dbName) {
    std::string filePath = catalogPath + "/SystemDatabases.bin";
    std::ofstream file(filePath, std::ios::binary | std::ios::app);
    if (!file.is_open()) return false;

    DBCatalogRecord rec;
    copyString(rec.name, dbName, 64);

    file.write(reinterpret_cast<const char*>(&rec), sizeof(DBCatalogRecord));
    file.close();
    return true;
}

bool SystemCatalog::addTable(const std::string& dbName, const std::string& tableName) {
    std::string filePath = catalogPath + "/SystemTables.bin";
    std::ofstream file(filePath, std::ios::binary | std::ios::app);
    if (!file.is_open()) return false;

    TableCatalogRecord rec;
    copyString(rec.dbName, dbName, 64);
    copyString(rec.tableName, tableName, 64);

    file.write(reinterpret_cast<const char*>(&rec), sizeof(TableCatalogRecord));
    file.close();
    return true;
}

bool SystemCatalog::addColumn(const std::string& dbName, const std::string& tableName, const std::string& colName, const std::string& type, size_t length) {
    std::string filePath = catalogPath + "/SystemColumns.bin";
    std::ofstream file(filePath, std::ios::binary | std::ios::app);
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

bool SystemCatalog::addIndex(const std::string& dbName, const std::string& tableName, const std::string& colName, const std::string& indexName, const std::string& indexType) {
    std::string filePath = catalogPath + "/SystemIndexes.bin";
    std::ofstream file(filePath, std::ios::binary | std::ios::app);
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

std::vector<std::string> SystemCatalog::getDatabases() {
    std::vector<std::string> dbs;
    std::string filePath = catalogPath + "/SystemDatabases.bin";
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) return dbs;

    DBCatalogRecord rec;
    while (file.read(reinterpret_cast<char*>(&rec), sizeof(DBCatalogRecord))) {
        dbs.push_back(std::string(rec.name));
    }
    file.close();
    return dbs;
}

std::vector<std::string> SystemCatalog::getTables(const std::string& dbName) {
    std::vector<std::string> tables;
    std::string filePath = catalogPath + "/SystemTables.bin";
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) return tables;

    TableCatalogRecord rec;
    while (file.read(reinterpret_cast<char*>(&rec), sizeof(TableCatalogRecord))) {
        if (std::string(rec.dbName) == dbName) {
            tables.push_back(std::string(rec.tableName));
        }
    }
    file.close();
    return tables;
}

std::vector<ColumnCatalogRecord> SystemCatalog::getColumns(const std::string& dbName, const std::string& tableName) {
    std::vector<ColumnCatalogRecord> columns;
    std::string filePath = catalogPath + "/SystemColumns.bin";
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) return columns;

    ColumnCatalogRecord rec;
    while (file.read(reinterpret_cast<char*>(&rec), sizeof(ColumnCatalogRecord))) {
        if (std::string(rec.dbName) == dbName && std::string(rec.tableName) == tableName) {
            columns.push_back(rec);
        }
    }
    file.close();
    return columns;
}

std::vector<IndexCatalogRecord> SystemCatalog::getIndexes() {
    std::vector<IndexCatalogRecord> indexes;
    std::string filePath = catalogPath + "/SystemIndexes.bin";
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) return indexes;

    IndexCatalogRecord rec;
    while (file.read(reinterpret_cast<char*>(&rec), sizeof(IndexCatalogRecord))) {
        indexes.push_back(rec);
    }
    file.close();
    return indexes;
}

std::vector<IndexCatalogRecord> SystemCatalog::getTableIndexes(const std::string& dbName, const std::string& tableName) {
    std::vector<IndexCatalogRecord> indexes;
    std::string filePath = catalogPath + "/SystemIndexes.bin";
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) return indexes;

    IndexCatalogRecord rec;
    while (file.read(reinterpret_cast<char*>(&rec), sizeof(IndexCatalogRecord))) {
        if (std::string(rec.dbName) == dbName && std::string(rec.tableName) == tableName) {
            indexes.push_back(rec);
        }
    }
    file.close();
    return indexes;
}