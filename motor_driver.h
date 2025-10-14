#include <WebServer.h>
#include <WebSocketsServer.h>
#include <Wifi.h>

//---- Motor Driver Configuration ------------

WebSocketsServer webSocket(85); // WebSocket on port 85
#define MOTOR_BAUD_RATE 115200

// --------- Setup Serial for UART ------------
void setupSerial() {
    Serial.begin(MOTOR_BAUD_RATE);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB
    }

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
        webSocket.sendTXT(num, "INFO: WiFi strength: " + String(WiFi.RSSI()) + "dBm");
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
            if (message.startsWith("HEARTBEAT")){
                webSocket.broadcastTXT(message + String(WiFi.RSSI()));
            } else {
                webSocket.broadcastTXT(message);
            }
        }
    }
}

void motor_websocket_server() {
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    MotorServer.begin();
    Serial.println("Motor WebSocket server started on port 85");
    Serial.println("WebSocket URL: ws://[ESP32_IP]:85");
}

void motor_server_handle_client() {
    webSocket.loop();           // Handle WebSocket events
    MotorServer.handleClient(); // Handle HTTP requests
    checkSTM32Messages();       // Check for STM32 responses
}
