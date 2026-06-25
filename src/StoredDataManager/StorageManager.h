#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <string>
#include <vector>

struct Record {
    size_t size;
    char* data;
    bool isDeleted;
};

class StorageManager {
private:
    std::string encryptionKey;

    void applyCipher(char* data, size_t size);
    std::string getTableFilePath(const std::string& dbName, const std::string& tableName);

public:
    StorageManager();
    ~StorageManager();

    bool createDatabase(const std::string& dbName);
    bool createTable(const std::string& dbName, const std::string& tableName);
    bool dropTable(const std::string& dbName, const std::string& tableName);

    std::vector<std::vector<std::string>> readAllRecords(
    const std::string& dbName,
    const std::string& tableName
    ); //mod de readallrecords
    //sebas
    bool insertRecord(
    const std::string& dbName,
    const std::string& tableName,
    const char* recordData,
    size_t size
);
    bool overwriteRecords(
    const std::string& dbName,
    const std::string& tableName,
    const std::vector<std::vector<std::string>>& records
    );
};

#endif