#include "StorageManager.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

StorageManager::StorageManager() {
    encryptionKey = "LlaveTinySQL";
}

StorageManager::~StorageManager() {
}

void StorageManager::applyCipher(char* data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        data[i] ^= encryptionKey[i % encryptionKey.length()];
    }
}

std::string StorageManager::getTableFilePath(const std::string& dbName, const std::string& tableName) {
    return dbName + "/" + tableName + ".bin";
}

bool StorageManager::createDatabase(const std::string& dbName) {
    if (!fs::exists(dbName)) {
        return fs::create_directory(dbName);
    }
    return false;
}

bool StorageManager::createTable(const std::string& dbName, const std::string& tableName) {
    std::string path = getTableFilePath(dbName, tableName);
    if (!fs::exists(path)) {
        std::ofstream file(path, std::ios::binary);
        file.close();
        return true;
    }
    return false;
}

bool StorageManager::dropTable(const std::string& dbName, const std::string& tableName) {
    std::string path = getTableFilePath(dbName, tableName);
    if (fs::exists(path)) {
        return fs::remove(path);
    }
    return false;
}

bool StorageManager::insertRecord(const std::string& dbName, const std::string& tableName, const char* recordData, size_t size) {
    std::string path = getTableFilePath(dbName, tableName);
    std::ofstream file(path, std::ios::binary | std::ios::app);
    if (!file.is_open()) return false;

    char* dataToSave = new char[size];
    for(size_t i = 0; i < size; i++) dataToSave[i] = recordData[i];

    applyCipher(dataToSave, size);

    file.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
    file.write(dataToSave, size);

    delete[] dataToSave;
    file.close();
    return true;
}

std::vector<Record> StorageManager::readAllRecords(const std::string& dbName, const std::string& tableName) {
    std::vector<Record> recordsList;
    std::string path = getTableFilePath(dbName, tableName);

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return recordsList;

    while (file.peek() != EOF) {
        Record rec;
        rec.isDeleted = false;

        if (!file.read(reinterpret_cast<char*>(&rec.size), sizeof(size_t))) {
            break;
        }

        rec.data = new char[rec.size];
        file.read(rec.data, rec.size);

        applyCipher(rec.data, rec.size);

        recordsList.push_back(rec);
    }

    file.close();
    return recordsList;
}