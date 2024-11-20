#include <Adafruit_MCP4728.h>
#include <Wire.h>
#include <math.h>

Adafruit_MCP4728 mcp1, mcp2;

const int PCA9548A_ADDRESS = 0x70;
const int CHANNEL_0 = 0x01;
const int CHANNEL_1 = 0x02;

const int pin9 = 9;
const int pin10 = 10;
const float voltageThreshold = 1.50;
const float vratio = 0.05; 

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Wire.begin();

  selectI2CChannel(CHANNEL_0);
  if (!mcp1.begin()) {
    Serial.println("Failed to find first MCP4728 chip");
    while (1) delay(10);
  }

  selectI2CChannel(CHANNEL_1);
  if (!mcp2.begin()) {
    Serial.println("Failed to find second MCP4728 chip");
    while (1) delay(10);
  }

  pinMode(pin9, OUTPUT);
  pinMode(pin10, OUTPUT);

  Serial.println("Ready");
}

void selectI2CChannel(uint8_t channel) {
  Wire.beginTransmission(PCA9548A_ADDRESS);
  Wire.write(channel);
  Wire.endTransmission();
}

void setVoltages(float voltage1, float voltage2, float voltage3, float voltage4, float voltage5, float voltage6, float voltage7, float voltage8) {
  selectI2CChannel(CHANNEL_0);
  mcp1.setChannelValue(MCP4728_CHANNEL_A, 4095 * (voltage1 / 5.0));
  mcp1.setChannelValue(MCP4728_CHANNEL_B, 4095 * (voltage2 / 5.0));
  mcp1.setChannelValue(MCP4728_CHANNEL_C, 4095 * (voltage3 / 5.0));
  mcp1.setChannelValue(MCP4728_CHANNEL_D, 4095 * (voltage4 / 5.0));

  selectI2CChannel(CHANNEL_1);
  mcp2.setChannelValue(MCP4728_CHANNEL_A, 4095 * (voltage5 / 5.0));
  mcp2.setChannelValue(MCP4728_CHANNEL_B, 4095 * (voltage6 / 5.0));
  mcp2.setChannelValue(MCP4728_CHANNEL_C, 4095 * (voltage7 / 5.0));
  mcp2.setChannelValue(MCP4728_CHANNEL_D, 4095 * (voltage8 / 5.0));
}

void loop() {
  if (Serial.available() > 0) {
    float x = Serial.parseFloat();
    float y = Serial.parseFloat();

    // First layer inferences are done in arduino 
    float hx = -0.77 * 8 * x - 0.07 * 8 * y - 1.01;
    float hy = 0.93 * 8 * x - 0.07 * 8 * y - 1.94;
    float hz = 0.32 * 8 * x + 0.86 * 8 * y + 1.88;
    float hw = -0.19 * 8 * x + 0.89 * 8 * y - 1.98;
    float sx = 1 / (1 + exp(-hx));
    float sy = 1 / (1 + exp(-hy));
    float sz = 1 / (1 + exp(-hz));
    float sw = 1 / (1 + exp(-hw));

    // Input voltages for computing using array 
    float voltage1 = 2*vratio; //level shift
    float voltage2 = 0;
    float voltage3 = sz*vratio; //positive weight
    float voltage4 = 0;
    float voltage5 = sx*vratio; //negative weight 1
    float voltage6 = sy*vratio; //negative weight 2
    float voltage7 = sw*vratio; //negative weight 3
    float voltage8 = 0;

    // Setting the voltages
    setVoltages(voltage1, voltage2, voltage3, voltage4, voltage5, voltage6, voltage7, voltage8);

    // Read output voltage using A0 and control Red & green LED to show the inference results using pin9 and pin10
    int sensorValue = analogRead(A0);
    float voltage = sensorValue * (5.0 / 1023.0);
    if (voltage > voltageThreshold) {
      digitalWrite(pin9, HIGH);
      digitalWrite(pin10, LOW);
      Serial.print("1,");
    } else {
      digitalWrite(pin9, LOW);
      digitalWrite(pin10, HIGH);
      Serial.print("0,");
    }
    Serial.println(voltage, 3);  // send the readout voltage
  }
  delay(10);
}
