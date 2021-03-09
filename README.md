# Telemedicine Temperature Checkup
## This system using ESP32 microcontroller

The data sent by device will be stored in the MongoDB database, and using REST API for method of sending data. Using the MLX90614 temperature sensor to read body temperature. 

0x1008 -> Body Temperature Characteristic <br />
0x1400 -> HDP Service <br /><br />

#define SERVICE_UUID (uint16_t)0x181A <br />
#define CHARACTERISTIC_UUID_RX "78604f25-789e-432e-b949-6fb2306fd5d7" <br />
#define CHARACTERISTIC_UUID_SSID "98a8d501-07ab-42a9-94e1-d590839bf71b" <br />
#define CHARACTERISTIC_UUID_PASS "2b9310ca-d12c-4940-a436-c33e32be84c7" <br />
#define CHARACTERISTIC_IV "ef70f86e-a5a4-499a-81e4-d8edffcd3a7e" <br />
#define CHARACTERISTIC_DATA "a8e6a804-216b-4dd8-90ff-a230226b42c1" <br />
