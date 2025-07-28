//proxy server

const fs = require("fs");
const https = require("https");
const WebSocket = require("ws");

const page = fs.readFileSync(`${__dirname}/proxyPage.html`);

//saving fake certificates (created with terminal command lines)
//to sslOptions object
const sslOptions = {
  key: fs.readFileSync("key.pem"),
  cert: fs.readFileSync("cert.pem"),
};

//super basic way of managing connections to this server
const server = https.createServer(sslOptions, (req, res) => {
  if (req.url === "/siglaBIM.png") {
    const img = fs.readFileSync(__dirname + "/siglaBIM.png");
    res.writeHead(200, { "Content-Type": "image/png" });
    res.end(img);
  } else if (req.url === "/siglaUMF.png") {
    const img = fs.readFileSync(__dirname + "/siglaUMF.png");
    res.writeHead(200, { "Content-Type": "image/png" });
    res.end(img);
  } else {
    res.end(page);
  }
});

//opening a websocket server on port 443 to communicate with Unity
const wssUnity = new WebSocket.Server({ server }); //443, where secured (ecnripted) data is served
//opening a websocket server on port 8081 to communicate with ESP32
const wssESP32 = new WebSocket.Server({ port: 8081 });

let unityClient = null;

//managing the connection of Unity game to this server
wssUnity.on("connection", (ws) => {
  console.log("Unity client connected via wss");
  unityClient = ws;
  ws.on("close", () => {
    unityClient = null;
  });
});

//managing the connection of ESP32 to this server
wssESP32.on("connection", (espSocket) => {
  espSocket.on("message", (msg) => {
    console.log("Data from ESP32:", msg.toString());
    if (unityClient && unityClient.readyState === WebSocket.OPEN) {
      const data = JSON.parse(msg.toString());
      unityClient.send(JSON.stringify({ Roll: data.Roll, Pitch: data.Pitch }));
    }
  });
});

//this server continuosly listents on port 443
server.listen(443, () => {
  console.log("HTTPS WebSocket server (for Unity) running on port 443");
});
