#include <WebServer.h>
#include <WebSocketsServer.h>

//---- Motor Driver Configuration ------------

WebServer MotorServer(84);
WebSocketsServer webSocket(85); // WebSocket on port 85
#define MOTOR_BAUD_RATE 115200

// --------- Setup Serial for UART ------------
void setupSerial() {
    Serial.begin(MOTOR_BAUD_RATE);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB
    }
    // Serial.println("Serial interface initialized");
}

//---- WebSocket Event Handler ------------

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    String command;
    switch (type) {
    case WStype_DISCONNECTED:
        // Serial.printf("[%u] Disconnected!\n", num);
        break;

    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        // Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
        // Send connection confirmation
        webSocket.sendTXT(num, "INFO: Connected to ESP32 Motor Controller");
        break;
    }

    case WStype_TEXT:
        // Serial.printf("[%u] Received: %s\n", num, payload);

        // Send command to STM32
        command = String((char *)payload);
        Serial.println(command);
        Serial.flush();

        // Send acknowledgment to client
        webSocket.sendTXT(num, "INFO: Command sent: " + command);
        break;

    default:
        break;
    }
}

//---- STM32 Response Handler ------------

void checkSTM32Messages() {
    // Check for incoming messages from STM32
    if (Serial.available()) {
        String message = "";

        // Read complete line from STM32
        while (Serial.available()) {
            char c = Serial.read();
            if (c == '\n' || c == '\r') {
                if (message.length() > 0) {
                    break;
                }
            } else {
                message += c;
            }
            delay(1);
        }

        // Broadcast STM32 response to all connected WebSocket clients
        if (message.length() > 0) {
            // Serial.println("STM32 Response: " + message);
            webSocket.broadcastTXT(message);
        }
    }
}

void motor_websocket_server() {
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    // Optional: Keep HTTP server for basic info page
    MotorServer.on("/", HTTP_GET, []() {
        String html = "<html><body>";
        html += "<h1>ESP32 Motor Controller</h1>";
        html += "<p>WebSocket Server running on port 85</p>";
        html += "<p>Connect via: ws://" + WiFi.localIP().toString() + ":85</p>";
        html += "<script>";
        html += "var ws = new WebSocket('ws://" + WiFi.localIP().toString() + ":85');";
        html += "ws.onmessage = function(event) { console.log('Received:', event.data); };";
        html += "function sendCommand(cmd) { ws.send(cmd); }";
        html += "</script>";
        html += "<button onclick=\"sendCommand('W,100,50')\">Forward</button>";
        html += "<button onclick=\"sendCommand('W,0,0')\">Stop</button>";
        html += "</body></html>";
        MotorServer.send(200, "text/html", html);
    });

    MotorServer.begin();

    Serial.println("Motor WebSocket server started on port 85");
    Serial.println("HTTP info page: http://[ESP32_IP]:84/");
    Serial.println("WebSocket URL: ws://[ESP32_IP]:85");
}

void motor_server_handle_client() {
    webSocket.loop();           // Handle WebSocket events
    MotorServer.handleClient(); // Handle HTTP requests
    checkSTM32Messages();       // Check for STM32 responses
}
