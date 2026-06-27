#include "QueryProcessor.h"

#include <algorithm>
#include <sstream>
#include <iostream>


using namespace std;

QueryProcessor::QueryProcessor() {
    currentDatabase = "";
}

string QueryProcessor::normalize(const string& query) {
    string normalized = query;

    while (!normalized.empty() &&
          (normalized.back() == ';' ||
           normalized.back() == '\n' ||
           normalized.back() == ' ')) {
        normalized.pop_back();
    }

    return normalized;
}

vector<string> QueryProcessor::split(const string& str, char delimiter) {
    vector<string> tokens;
    string token;
    stringstream ss(str);

    while (getline(ss, token, delimiter)) {
        if (!token.empty()) {
            token.erase(0, token.find_first_not_of(" "));
            token.erase(token.find_last_not_of(" ") + 1);
            tokens.push_back(token);
        }
    }

    return tokens;
}

QueryResult QueryProcessor::execute(const string& query) {
    auto start = chrono::high_resolution_clock::now();

    string normalized = normalize(query);
    QueryResult result;

    if (normalized.find("CREATE DATABASE") == 0) {
        result = handleCreateDatabase(normalized);
    }
    else if (normalized.find("SET DATABASE") == 0) {
        result = handleSetDatabase(normalized);
    }
    else if (normalized.find("CREATE TABLE") == 0) {
        result = handleCreateTable(normalized);
    }
    else if (normalized.find("INSERT INTO") == 0) {
        result = handleInsert(normalized);
    }
    else if (normalized.find("SELECT") == 0) {
        result = handleSelect(normalized);
    }
    else if (normalized.find("UPDATE") == 0) {
        result = handleUpdate(normalized);
    }
    else if (normalized.find("DELETE FROM") == 0) {
        result = handleDelete(normalized);
    }
    else if (normalized.find("CREATE INDEX") == 0) {
        result = handleCreateIndex(normalized);
    }
    else if (normalized.find("DROP TABLE") == 0) {
    result = handleDropTable(query);
    }
    else {
        result.success = false;
        result.message = "Unknown SQL statement";
    }
    

    auto end = chrono::high_resolution_clock::now();

    result.executionTime =
        chrono::duration<double, milli>(end - start).count();

    return result;
}

QueryResult QueryProcessor::handleCreateDatabase(const string& query) {
    QueryResult result;

    vector<string> tokens = split(query, ' ');

    if (tokens.size() < 3) {
        result.success = false;
        result.message = "Invalid CREATE DATABASE syntax";
        return result;
    }

    string dbName = tokens[2];

    if (!storage.createDatabase(dbName)) {
        result.success = false;
        result.message = "Could not create database";
        return result;
    }

    catalog.addDatabase(dbName);

    result.success = true;
    result.message = "Database created successfully";

    return result;
}

QueryResult QueryProcessor::handleSetDatabase(const string& query) {
    QueryResult result;

    vector<string> tokens = split(query, ' ');

    if (tokens.size() < 3) {
        result.success = false;
        result.message = "Invalid SET DATABASE syntax";
        return result;
    }

    string dbName = tokens[2];

    vector<string> databases = catalog.getDatabases();

    bool found = false;

    for (const auto& db : databases) {
        if (db == dbName) {
            found = true;
            break;
        }
    }

    if (!found) {
        result.success = false;
        result.message = "Database does not exist";
        return result;
    }

    currentDatabase = dbName;

    result.success = true;
    result.message = "Database selected: " + dbName;

    return result;
}

QueryResult QueryProcessor::handleCreateTable(const string& query) {
    QueryResult result;

    if (currentDatabase.empty()) {
        result.success = false;
        result.message = "No database selected";
        return result;
    }

    size_t tablePos = query.find("CREATE TABLE");
    size_t openParen = query.find("(");
    size_t closeParen = query.rfind(")");

    if (openParen == string::npos || closeParen == string::npos) {
        result.success = false;
        result.message = "Invalid CREATE TABLE syntax";
        return result;
    }

    string tableName =
        query.substr(tablePos + 13, openParen - (tablePos + 13));

    tableName.erase(0, tableName.find_first_not_of(" "));
    tableName.erase(tableName.find_last_not_of(" ") + 1);

    string columnsBlock =
        query.substr(openParen + 1, closeParen - openParen - 1);

    vector<string> columnDefs = split(columnsBlock, ',');

    if (!storage.createTable(currentDatabase, tableName)) {
        result.success = false;
        result.message = "Could not create table";
        return result;
    }

    catalog.addTable(currentDatabase, tableName);

    for (const auto& colDef : columnDefs) {
        vector<string> parts = split(colDef, ' ');

        if (parts.size() < 2) {
            result.success = false;
            result.message = "Invalid column definition";
            return result;
        }

        string columnName = parts[0];
        string columnType = parts[1];
        size_t length = 0;

        // Detectar VARCHAR(n)
        if (columnType.find("VARCHAR") == 0) {
            size_t open = columnType.find("(");
            size_t close = columnType.find(")");

            if (open != string::npos && close != string::npos) {
                length = stoi(
                    columnType.substr(open + 1, close - open - 1)
                );
            }
        }

        catalog.addColumn(
            currentDatabase,
            tableName,
            columnName,
            columnType,
            length
        );
    }

    result.success = true;
    result.message = "Table created successfully";

    return result;
}

QueryResult QueryProcessor::handleInsert(const string& query) {
    QueryResult result;

    if (currentDatabase.empty()) {
        result.success = false;
        result.message = "No database selected";
        return result;
    }

    size_t intoPos = query.find("INSERT INTO");
    size_t valuesPos = query.find("VALUES");
    size_t openParen = query.find("(", valuesPos);
    size_t closeParen = query.rfind(")");

    if (valuesPos == string::npos ||
        openParen == string::npos ||
        closeParen == string::npos) {
        result.success = false;
        result.message = "Invalid INSERT syntax";
        return result;
    }

    string tableName =
        query.substr(intoPos + 11, valuesPos - (intoPos + 11));

    tableName.erase(0, tableName.find_first_not_of(" "));
    tableName.erase(tableName.find_last_not_of(" ") + 1);

    string valuesBlock =
        query.substr(openParen + 1, closeParen - openParen - 1);

    vector<string> values = split(valuesBlock, ',');

    // limpiar comillas y espacios
    for (auto& value : values) {
        value.erase(0, value.find_first_not_of(" "));
        value.erase(value.find_last_not_of(" ") + 1);

        if (!value.empty() && value.front() == '"')
            value.erase(0, 1);

        if (!value.empty() && value.back() == '"')
            value.pop_back();
    }

    vector<ColumnCatalogRecord> columns =
        catalog.getColumns(currentDatabase, tableName);

    if (columns.empty()) {
        result.success = false;
        result.message = "Table does not exist";
        return result;
    }

    if (values.size() != columns.size()) {
        result.success = false;
        result.message = "Column count does not match";
        return result;
    }

    // Validación de tipos
    for (size_t i = 0; i < values.size(); i++) {
        string type = columns[i].type;

        if (type == "INTEGER") {
            try { stoi(values[i]); }
            catch (...) {
                result.success = false;
                result.message = "Invalid INTEGER value";
                return result;
            }
        }
        else if (type == "DOUBLE") {
            try { stod(values[i]); }
            catch (...) {
                result.success = false;
                result.message = "Invalid DOUBLE value";
                return result;
            }
        }
        else if (type.find("VARCHAR") == 0) {
            // válido como string
        }
        else if (type == "DATETIME") {
            // luego podemos hacer validación más estricta
        }
    }

    string serialized = "";

    for (size_t i = 0; i < values.size(); i++) {
        serialized += values[i];

        if (i < values.size() - 1) {
            serialized += "|";
        }
    }

    if (!storage.insertRecord(
            currentDatabase,
            tableName,
            serialized.c_str(),
            serialized.size()
        )) {
        result.success = false;
        result.message = "Failed to insert record";
        return result;
    }

    result.success = true;
    result.message = "Record inserted successfully";

    return result;
}

QueryResult QueryProcessor::handleSelect(const string& query) {
    QueryResult result;

    if (currentDatabase.empty()) {
        result.success = false;
        result.message = "No database selected";
        return result;
    }

    size_t fromPos = query.find("FROM");

    if (fromPos == string::npos) {
        result.success = false;
        result.message = "Invalid SELECT syntax";
        return result;
    }

    string selectedColumns =
        query.substr(6, fromPos - 6);

    selectedColumns.erase(0, selectedColumns.find_first_not_of(" "));
    selectedColumns.erase(selectedColumns.find_last_not_of(" ") + 1);

    size_t wherePos = query.find("WHERE");
    size_t orderPos = query.find("ORDER BY");

    string tableName;

    if (wherePos != string::npos)
        tableName = query.substr(fromPos + 4, wherePos - (fromPos + 4));
    else if (orderPos != string::npos)
        tableName = query.substr(fromPos + 4, orderPos - (fromPos + 4));
    else
        tableName = query.substr(fromPos + 4);

    tableName.erase(0, tableName.find_first_not_of(" "));
    tableName.erase(tableName.find_last_not_of(" ") + 1);

    vector<vector<string>> rows =
        storage.readAllRecords(currentDatabase, tableName);

    if (rows.empty()) {
        result.success = true;
        result.message = "No records found";
        return result;
    }

    vector<ColumnCatalogRecord> columns =
        catalog.getColumns(currentDatabase, tableName);

    // WHERE
    if (wherePos != string::npos) {
        string whereClause;

        if (orderPos != string::npos)
            whereClause = query.substr(
                wherePos + 5,
                orderPos - (wherePos + 5)
            );
        else
            whereClause = query.substr(wherePos + 5);

        whereClause.erase(0, whereClause.find_first_not_of(" "));
        whereClause.erase(whereClause.find_last_not_of(" ") + 1);

        vector<string> parts = split(whereClause, '=');

        if (parts.size() == 2) {
            string whereColumn = parts[0];
            string whereValue = parts[1];

            whereColumn.erase(0, whereColumn.find_first_not_of(" "));
            whereColumn.erase(whereColumn.find_last_not_of(" ") + 1);

            whereValue.erase(0, whereValue.find_first_not_of(" "));
            whereValue.erase(whereValue.find_last_not_of(" ") + 1);

            int columnIndex = -1;

            for (int i = 0; i < columns.size(); i++) {
                if (columns[i].columnName == whereColumn) {
                    columnIndex = i;
                    break;
                }
            }

            if (columnIndex == -1) {
                result.success = false;
                result.message = "Invalid WHERE column";
                return result;
            }

            vector<vector<string>> filteredRows;

            string key =
                currentDatabase + "." + tableName + "." + whereColumn;

            if (bstIndexes.find(key) != bstIndexes.end()) {
                cout << "Using BST index..." << endl;

                long long pos =
                    bstIndexes[key].search(whereValue);

                if (pos != -1) {
                    filteredRows.push_back(rows[pos]);
                }
            }
            else if (btreeIndexes.find(key) != btreeIndexes.end()) {
                cout << "Using BTree index..." << endl;

                long long pos =
                    btreeIndexes[key].search(whereValue);

                if (pos != -1) {
                    filteredRows.push_back(rows[pos]);
                }
            }
            else {
                cout << "Using full scan..." << endl;

                for (const auto& row : rows) {
                    if (row[columnIndex] == whereValue) {
                        filteredRows.push_back(row);
                    }
                }
            }

            rows = filteredRows;
        }
    }

    // ORDER BY
    if (orderPos != string::npos) {
        string orderClause = query.substr(orderPos + 8);

        vector<string> parts = split(orderClause, ' ');

        if (parts.size() >= 1) {
            string orderColumn = parts[0];
            bool asc = true;

            if (parts.size() >= 2) {
                asc = (parts[1] != "DESC");
            }

            int columnIndex = -1;

            for (int i = 0; i < columns.size(); i++) {
                if (columns[i].columnName == orderColumn) {
                    columnIndex = i;
                    break;
                }
            }

            if (columnIndex == -1) {
                result.success = false;
                result.message = "Invalid ORDER BY column";
                return result;
            }
            cout << "Using QuickSort..." << endl;

            quickSort(
                rows,
                0,
                rows.size() - 1,
                columnIndex,
                asc
            );
        }
    }

    // Selección de columnas
    if (selectedColumns != "*") {
        vector<string> requestedColumns =
            split(selectedColumns, ',');

        vector<int> indexes;

        for (const auto& col : requestedColumns) {
            for (int i = 0; i < columns.size(); i++) {
                if (columns[i].columnName == col) {
                    indexes.push_back(i);
                }
            }
        }

        vector<vector<string>> projectedRows;

        for (const auto& row : rows) {
            vector<string> newRow;

            for (int idx : indexes) {
                newRow.push_back(row[idx]);
            }

            projectedRows.push_back(newRow);
        }

        rows = projectedRows;
    }

    result.success = true;
    result.message = "Query executed successfully";
    result.rows = rows;

    return result;
}

void QueryProcessor::quickSort(
    vector<vector<string>>& rows,
    int low,
    int high,
    int columnIndex,
    bool asc
) {
    if (low < high) {
        int pi = partition(
            rows,
            low,
            high,
            columnIndex,
            asc
        );

        quickSort(rows, low, pi - 1, columnIndex, asc);
        quickSort(rows, pi + 1, high, columnIndex, asc);
    }
}

int QueryProcessor::partition(
    vector<vector<string>>& rows,
    int low,
    int high,
    int columnIndex,
    bool asc
) {
    string pivot = rows[high][columnIndex];
    int i = low - 1;

    for (int j = low; j < high; j++) {
        bool condition;

        if (asc)
            condition = rows[j][columnIndex] < pivot;
        else
            condition = rows[j][columnIndex] > pivot;

        if (condition) {
            i++;
            swap(rows[i], rows[j]);
        }
    }

    swap(rows[i + 1], rows[high]);

    return i + 1;
}

QueryResult QueryProcessor::handleUpdate(const string& query) {
    QueryResult result;

    if (currentDatabase.empty()) {
        result.success = false;
        result.message = "No database selected";
        return result;
    }

    size_t setPos = query.find("SET");
    size_t wherePos = query.find("WHERE");

    if (setPos == string::npos) {
        result.success = false;
        result.message = "Invalid UPDATE syntax";
        return result;
    }

    string tableName =
        query.substr(6, setPos - 6);

    tableName.erase(0, tableName.find_first_not_of(" "));
    tableName.erase(tableName.find_last_not_of(" ") + 1);

    string setClause;
    string whereClause;

    if (wherePos != string::npos) {
        setClause =
            query.substr(setPos + 3, wherePos - (setPos + 3));
        whereClause =
            query.substr(wherePos + 5);
    } else {
        setClause =
            query.substr(setPos + 3);
    }

    setClause.erase(0, setClause.find_first_not_of(" "));
    setClause.erase(setClause.find_last_not_of(" ") + 1);

    vector<string> setParts = split(setClause, '=');

    if (setParts.size() != 2) {
        result.success = false;
        result.message = "Invalid SET clause";
        return result;
    }

    string targetColumn = setParts[0];
    string newValue = setParts[1];

    targetColumn.erase(0, targetColumn.find_first_not_of(" "));
    targetColumn.erase(targetColumn.find_last_not_of(" ") + 1);

    newValue.erase(0, newValue.find_first_not_of(" "));
    newValue.erase(newValue.find_last_not_of(" ") + 1);

    if (!newValue.empty() && newValue.front() == '"')
        newValue.erase(0, 1);

    if (!newValue.empty() && newValue.back() == '"')
        newValue.pop_back();

    vector<ColumnCatalogRecord> columns =
        catalog.getColumns(currentDatabase, tableName);

    vector<vector<string>> rows =
        storage.readAllRecords(currentDatabase, tableName);

    if (rows.empty()) {
        result.success = false;
        result.message = "No records found";
        return result;
    }

    int targetIndex = -1;

    for (int i = 0; i < columns.size(); i++) {
        if (columns[i].columnName == targetColumn) {
            targetIndex = i;
            break;
        }
    }

    if (targetIndex == -1) {
        result.success = false;
        result.message = "Invalid column in SET";
        return result;
    }

    int updatedCount = 0;

    if (wherePos != string::npos) {
        whereClause.erase(0, whereClause.find_first_not_of(" "));
        whereClause.erase(whereClause.find_last_not_of(" ") + 1);

        vector<string> whereParts = split(whereClause, '=');

        if (whereParts.size() != 2) {
            result.success = false;
            result.message = "Invalid WHERE clause";
            return result;
        }

        string whereColumn = whereParts[0];
        string whereValue = whereParts[1];

        whereColumn.erase(0, whereColumn.find_first_not_of(" "));
        whereColumn.erase(whereColumn.find_last_not_of(" ") + 1);

        whereValue.erase(0, whereValue.find_first_not_of(" "));
        whereValue.erase(whereValue.find_last_not_of(" ") + 1);

        int whereIndex = -1;

        for (int i = 0; i < columns.size(); i++) {
            if (columns[i].columnName == whereColumn) {
                whereIndex = i;
                break;
            }
        }

        if (whereIndex == -1) {
            result.success = false;
            result.message = "Invalid WHERE column";
            return result;
        }

        for (auto& row : rows) {
            if (row[whereIndex] == whereValue) {
                row[targetIndex] = newValue;
                updatedCount++;
            }
        }
    } else {
        for (auto& row : rows) {
            row[targetIndex] = newValue;
            updatedCount++;
        }
    }

    storage.overwriteRecords(currentDatabase, tableName, rows);

    result.success = true;
    result.message =
        to_string(updatedCount) + " record(s) updated";

    return result;
}

QueryResult QueryProcessor::handleDelete(const string& query) {
    QueryResult result;

    if (currentDatabase.empty()) {
        result.success = false;
        result.message = "No database selected";
        return result;
    }

    size_t fromPos = query.find("FROM");
    size_t wherePos = query.find("WHERE");

    if (fromPos == string::npos) {
        result.success = false;
        result.message = "Invalid DELETE syntax";
        return result;
    }

    string tableName;

    if (wherePos != string::npos)
        tableName =
            query.substr(fromPos + 4, wherePos - (fromPos + 4));
    else
        tableName =
            query.substr(fromPos + 4);

    tableName.erase(0, tableName.find_first_not_of(" "));
    tableName.erase(tableName.find_last_not_of(" ") + 1);

    vector<ColumnCatalogRecord> columns =
        catalog.getColumns(currentDatabase, tableName);

    vector<vector<string>> rows =
        storage.readAllRecords(currentDatabase, tableName);

    if (rows.empty()) {
        result.success = false;
        result.message = "No records found";
        return result;
    }

    int deletedCount = 0;

    // DELETE con WHERE
    if (wherePos != string::npos) {
        string whereClause = query.substr(wherePos + 5);

        whereClause.erase(0, whereClause.find_first_not_of(" "));
        whereClause.erase(whereClause.find_last_not_of(" ") + 1);

        vector<string> whereParts = split(whereClause, '=');

        if (whereParts.size() != 2) {
            result.success = false;
            result.message = "Invalid WHERE clause";
            return result;
        }

        string whereColumn = whereParts[0];
        string whereValue = whereParts[1];

        whereColumn.erase(0, whereColumn.find_first_not_of(" "));
        whereColumn.erase(whereColumn.find_last_not_of(" ") + 1);

        whereValue.erase(0, whereValue.find_first_not_of(" "));
        whereValue.erase(whereValue.find_last_not_of(" ") + 1);

        int whereIndex = -1;

        for (int i = 0; i < columns.size(); i++) {
            if (columns[i].columnName == whereColumn) {
                whereIndex = i;
                break;
            }
        }

        if (whereIndex == -1) {
            result.success = false;
            result.message = "Invalid WHERE column";
            return result;
        }

        vector<vector<string>> remainingRows;

        for (const auto& row : rows) {
            if (row[whereIndex] == whereValue) {
                deletedCount++;
            } else {
                remainingRows.push_back(row);
            }
        }

        rows = remainingRows;
    }
    else {
        // DELETE sin WHERE = borra todo
        deletedCount = rows.size();
        rows.clear();
    }

    if (!storage.overwriteRecords(currentDatabase, tableName, rows)) {
        result.success = false;
        result.message = "Failed to delete records";
        return result;
    }

    result.success = true;
    result.message =
        to_string(deletedCount) + " record(s) deleted";

    return result;
}

QueryResult QueryProcessor::handleCreateIndex(const string& query) {
    QueryResult result;

    if (currentDatabase.empty()) {
        result.success = false;
        result.message = "No database selected";
        return result;
    }

    size_t onPos = query.find("ON");
    size_t openParen = query.find("(");
    size_t closeParen = query.find(")");
    size_t typePos = query.find("USING");

    if (
        onPos == string::npos ||
        openParen == string::npos ||
        closeParen == string::npos ||
        typePos == string::npos
    ) {
        result.success = false;
        result.message = "Invalid CREATE INDEX syntax";
        return result;
    }

    string indexName =
        query.substr(13, onPos - 13);

    indexName.erase(0, indexName.find_first_not_of(" "));
    indexName.erase(indexName.find_last_not_of(" ") + 1);

    string tableName =
        query.substr(onPos + 2, openParen - (onPos + 2));

    tableName.erase(0, tableName.find_first_not_of(" "));
    tableName.erase(tableName.find_last_not_of(" ") + 1);

    string columnName =
        query.substr(openParen + 1, closeParen - openParen - 1);

    string indexType =
        query.substr(typePos + 5);

    indexType.erase(0, indexType.find_first_not_of(" "));
    indexType.erase(indexType.find_last_not_of(" ") + 1);

    vector<ColumnCatalogRecord> columns =
        catalog.getColumns(currentDatabase, tableName);

    int columnIndex = -1;

    for (int i = 0; i < columns.size(); i++) {
        if (columns[i].columnName == columnName) {
            columnIndex = i;
            break;
        }
    }

    if (columnIndex == -1) {
        result.success = false;
        result.message = "Column does not exist";
        return result;
    }

    vector<vector<string>> rows =
        storage.readAllRecords(currentDatabase, tableName);

    string key =
        currentDatabase + "." + tableName + "." + columnName;

    if (indexType == "BST") {
        IndexBST bst;

        for (int i = 0; i < rows.size(); i++) {
            bst.insert(rows[i][columnIndex], i);
        }

        bstIndexes[key] = bst;
    }
    else if (indexType == "BTREE") {
        IndexBTree btree;

        for (int i = 0; i < rows.size(); i++) {
            btree.insert(rows[i][columnIndex], i);
        }

        btreeIndexes[key] = btree;
    }
    else {
        result.success = false;
        result.message = "Invalid index type";
        return result;
    }

    catalog.addIndex(
        currentDatabase,
        tableName,
        indexName,
        columnName,
        indexType
    );

    result.success = true;
    result.message = "Index created successfully";

    return result;
}

QueryResult QueryProcessor::handleDropTable(const string& query) {
    QueryResult result;

    if (currentDatabase.empty()) {
        result.success = false;
        result.message = "No database selected";
        return result;
    }

    vector<string> parts = split(query, ' ');

    if (parts.size() < 3) {
        result.success = false;
        result.message = "Invalid DROP TABLE syntax";
        return result;
    }

    string tableName = parts[2];

    if (!tableName.empty() && tableName.back() == ';') {
        tableName.pop_back();
    }

    if (!storage.dropTable(currentDatabase, tableName)) {
        result.success = false;
        result.message = "Table does not exist";
        return result;
    }

    result.success = true;
    result.message = "Table dropped successfully";

    return result;
}