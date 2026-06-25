#include "StorageManager.h"
#include <fstream>
#include <filesystem>

using namespace std;

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

vector<vector<string>> StorageManager::readAllRecords(
    const string& dbName,
    const string& tableName
) {
    vector<vector<string>> recordsList;
    string path = getTableFilePath(dbName, tableName);

    ifstream file(path, ios::binary);
    if (!file.is_open()) return recordsList;

    while (file.peek() != EOF) {
        size_t size;

        if (!file.read(reinterpret_cast<char*>(&size), sizeof(size_t))) {
            break;
        }

        char* buffer = new char[size];
        file.read(buffer, size);

        applyCipher(buffer, size);

        string recordData(buffer, size);

        delete[] buffer;

        vector<string> parsedRecord;
        string token = "";

        for (char c : recordData) {
            if (c == '|') {
                parsedRecord.push_back(token);
                token = "";
            } else {
                token += c;
            }
        }

        parsedRecord.push_back(token);

        recordsList.push_back(parsedRecord);
    }

    file.close();
    return recordsList;
}

bool StorageManager::overwriteRecords(
    const string& dbName,
    const string& tableName,
    const vector<vector<string>>& records
) {
    string path = getTableFilePath(dbName, tableName);

    ofstream file(path, ios::binary | ios::trunc);

    if (!file.is_open()) {
        return false;
    }

    for (const auto& record : records) {
        string serialized = "";

        for (size_t i = 0; i < record.size(); i++) {
            serialized += record[i];

            if (i < record.size() - 1) {
                serialized += "|";
            }
        }

        char* dataToSave = new char[serialized.size()];

        for (size_t i = 0; i < serialized.size(); i++) {
            dataToSave[i] = serialized[i];
        }

        applyCipher(dataToSave, serialized.size());

        size_t size = serialized.size();

        file.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
        file.write(dataToSave, size);

        delete[] dataToSave;
    }

    file.close();
    return true;
}