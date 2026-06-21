#include "StorageManager.h"
#include <fstream>
#include <filesystem>

using namespace std;

namespace fs = filesystem;

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

string StorageManager::getTableFilePath(const string& dbName, const string& tableName) {
    return dbName + "/" + tableName + ".bin";
}

bool StorageManager::createDatabase(const string& dbName) {
    if (!fs::exists(dbName)) {
        return fs::create_directory(dbName);
    }
    return false;
}

bool StorageManager::createTable(const string& dbName, const string& tableName) {
    string path = getTableFilePath(dbName, tableName);
    if (!fs::exists(path)) {
        ofstream file(path, ios::binary);
        file.close();
        return true;
    }
    return false;
}

bool StorageManager::dropTable(const string& dbName, const string& tableName) {
    string path = getTableFilePath(dbName, tableName);
    if (fs::exists(path)) {
        return fs::remove(path);
    }
    return false;
}

bool StorageManager::insertRecord(const string& dbName, const string& tableName, const char* recordData, size_t size) {
    string path = getTableFilePath(dbName, tableName);
    ofstream file(path, ios::binary | ios::app);
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

vector<Record> StorageManager::readAllRecords(const string& dbName, const string& tableName) {
    vector<Record> recordsList;
    string path = getTableFilePath(dbName, tableName);

    ifstream file(path, ios::binary);
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