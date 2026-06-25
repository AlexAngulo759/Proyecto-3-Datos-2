document
    .getElementById("executeBtn")
    .addEventListener("click", executeQuery);

async function executeQuery() {
    const query = document.getElementById("queryInput").value;

    const button = document.getElementById("executeBtn");
    button.innerText = "Executing...";

    try {
        const response = await fetch("http://127.0.0.1:8080/execute", {
            method: "POST",
            body: query
        });

        const data = await response.json();

        document.getElementById("message").innerText = data.message;
        document.getElementById("executionTime").innerText =
            data.executionTime + " ms";

        const table = document.getElementById("resultTable");
        table.innerHTML = "";

        if (data.rows && data.rows.length > 0) {
            for (let row of data.rows) {
                const tr = document.createElement("tr");

                for (let cell of row) {
                    const td = document.createElement("td");
                    td.innerText = cell;
                    tr.appendChild(td);
                }

                table.appendChild(tr);
            }
        }

    } catch (error) {
        document.getElementById("message").innerText =
            "Connection error with TinySQL Server";
    }

    button.innerText = "Execute Query";
}