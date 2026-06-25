#include "ApiServer.h"
#include "../../include/httplib.h"
#include <iostream>

using namespace httplib;

ApiServer::ApiServer() {
}

void ApiServer::start(int port) {
    Server server;

    server.Post("/execute", [this](const Request& req, Response& res) {
        std::string query = req.body;

        QueryResult result = queryProcessor.execute(query);

        std::string json = "{";
        json += "\"success\": " + std::string(result.success ? "true" : "false") + ",";
        json += "\"message\": \"" + result.message + "\",";
        json += "\"executionTime\": " + std::to_string(result.executionTime) + ",";
        json += "\"rows\": [";

        for (size_t i = 0; i < result.rows.size(); i++) {
            json += "[";

            for (size_t j = 0; j < result.rows[i].size(); j++) {
                json += "\"" + result.rows[i][j] + "\"";

                if (j < result.rows[i].size() - 1)
                    json += ",";
            }

            json += "]";

            if (i < result.rows.size() - 1)
                json += ",";
        }

        json += "]}";

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");

        res.set_content(json, "application/json");
    });

    std::cout << "Server running on port " << port << std::endl;
    server.listen("0.0.0.0", port);
}