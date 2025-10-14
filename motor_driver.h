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
    static char message_buf[128];
    static uint8_t message_pos = 0;

    while (Serial.available()) {
        char in_char = Serial.read();

        if (in_char == '\n' || in_char == '\r') { // End of line detected
            if (message_pos > 0) {
                message_buf[message_pos] = '\0'; // Null-terminate the string
                String message_str = String(message_buf);

                if (message_str.startsWith("HEARTBEAT")) {
                    webSocket.broadcastTXT(message_str + String(WiFi.RSSI()));
                } else {
                    webSocket.broadcastTXT(message_str);
                }
                message_pos = 0; // Reset for the next message
            }
        } else if (message_pos < sizeof(message_buf) - 1) {
            message_buf[message_pos++] = in_char;
        }
    }
}

void motor_websocket_server() {
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("Motor WebSocket server started on port 85");
    Serial.println("WebSocket URL: ws://[ESP32_IP]:85");
}

void motor_server_handle_client() {
    webSocket.loop();           // Handle WebSocket events
    checkSTM32Messages();       // Check for STM32 responses
}
