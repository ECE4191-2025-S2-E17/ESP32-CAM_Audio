#include <WebServer.h>

//---- Motor Driver Configuration ------------

WebServer MotorServer(84);
#define MOTOR_BAUD_RATE 115200

// --------- Setup Serial for UART ------------
void setupSerial() {
    Serial.begin(MOTOR_BAUD_RATE);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB
    }
    Serial.println("Serial interface initialized");
}

//---- Motor Control Functions ------------

void handleDriveCommand() {

    // Get the lw and rw parameters from the URL
    String lwParam = MotorServer.arg("lw");
    String rwParam = MotorServer.arg("rw");

    // Convert parameters to integers
    int leftWheel = lwParam.toInt();
    int rightWheel = rwParam.toInt();

    // Print the received commands to serial with command identifier for STM32
    Serial.print("W,"); // W = Wheel command identifier
    Serial.print(leftWheel);
    Serial.print(",");
    Serial.println(rightWheel);

    // Debug output (optional - comment out for production)
    // Serial.print("Debug - Motor Command - Left Wheel: ");
    // Serial.print(leftWheel);
    // Serial.print(", Right Wheel: ");
    // Serial.println(rightWheel);

    // Send response back to client
    String response = "OK - Left: " + String(leftWheel) + ", Right: " + String(rightWheel);
    MotorServer.send(200, "text/plain", response);
}

void motor_http_server() {

    MotorServer.on("/drive", HTTP_GET, handleDriveCommand);
    MotorServer.begin();

    Serial.println("Motor driver server started on port 84");
    Serial.println("Usage: http://[ESP32_IP]:84/drive?lw=[left_wheel_value]&rw=[right_wheel_value]");
}
