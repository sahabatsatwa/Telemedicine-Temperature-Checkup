#include <Arduino.h>
#include <sstream>
#include <Crypto.h>
#include <base64.hpp>
#include <SparkFunMLX90614.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_SSD1306.h>

#define BLOCK_SIZE 32
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

const char* ssid     = "MBC Laboratory.";
const char* password = "123gogoans";
const String server = "159.89.204.122";
const String port = "5000";

IRTherm temp;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
HTTPClient http;

typedef uint8_t byte;
uint32_t Ka;
uint8_t iv[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
byte shaResult[SHA256_SIZE];
const uint32_t prime = 2147483647;
const uint32_t generator = 16807;
int count = 1;
uint32_t B = 0;
float objectT;
float ambientT;
uint32_t a;
uint32_t A;

void wifi_connect() 
{
  display.clearDisplay();
  display.setCursor(0, 20);
  display.println("Connecting to ");
  display.setCursor(0, 30);
  display.println(ssid);
  display.display();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  display.clearDisplay();
  display.setCursor(0, 20);
  display.println("WiFi connected.");
  display.setCursor(0, 30);
  display.print("IP address: ");
  display.println(WiFi.localIP());
  display.display();
  delay(5000);
}

uint32_t keyGen() 
{
  return random(1, prime);
}

uint32_t mul_mod(uint32_t a, uint32_t b, uint32_t m)
{
  uint32_t result = 0; //variable to store the result
  uint32_t runningCount = b % m; //holds the value of b*2^i

  for (int i = 0 ; i < 32 ; i++) {
    if (i > 0) runningCount = (runningCount << 1) % m;
    if (bitRead(a, i)) {
      result = (result % m + runningCount % m) % m;
    }
  }
  return result;
}

uint32_t pow_mod(uint32_t b, uint32_t e, uint32_t m)
{
  uint32_t r;  // result of this function
  uint32_t pow;
  uint32_t e_i = e;
  uint8_t i;

  if ( b == 0 || m == 0 ) {
    return 0;
  }

  if ( e == 0 ) {
    return 1;
  }

  b = b % m;
  pow = b;
  r = 1;

  while ( e_i ) {
    if ( e_i & 1 ) {
      r = mul_mod(r, pow, m); //(r * pow) % m;
    }
    pow = mul_mod(pow, pow, m); //(pow * pow) % m;
    e_i = e_i >> 1;
    i++;
  }
  return r;
}

void bufferSize(char* text, int &length)
{
  int i = strlen(text);
  int buf = round(i / BLOCK_SIZE) * BLOCK_SIZE;
  length = (buf <= i) ? buf + BLOCK_SIZE : length = buf;
}

void encrypt(char* plain_text, char* output, int length)
{
  byte enciphered[length];
  int in_size;
  byte suhu[in_size];
  AES aesEncryptor(shaResult, iv, AES::AES_MODE_256, AES::CIPHER_ENCRYPT);
  //aesEncryptor.calcSizeAndPad(in_size);
  //aesEncryptor.padPlaintext((uint8_t*)plain_text, suhu);
  aesEncryptor.process((uint8_t*)plain_text, enciphered, length);
  int encrypted_size = sizeof(enciphered);
  char encoded[encrypted_size];
  encode_base64(enciphered, encrypted_size, (unsigned char*)encoded);
  strcpy(output, encoded);
}

void decrypt(char* enciphered, char* output, int length)
{
  length = length; //re-adjust
  char decoded[length];
  decode_base64((unsigned char*)enciphered, (unsigned char*)decoded);
  bufferSize(enciphered, length);
  byte deciphered[length];
  AES aesDecryptor(shaResult, iv, AES::AES_MODE_256, AES::CIPHER_DECRYPT);
  aesDecryptor.process((uint8_t*)decoded, deciphered, length);
  strcpy(output, (char*)deciphered);
}

void hashing(char* payload)
{
  SHA256 hasher;
  hasher.doUpdate(payload, strlen(payload));
  hasher.doFinal(shaResult);
  
}

void postData(char* data) 
{
  String url = "http://" + server + ":" + port + "/input/suhu";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  String suhu = data;
  Serial.println(suhu);
  int httpResponseCode = http.POST("{\"Suhu (celcius)\":\"" + suhu + "\"}");
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
  } else {
    Serial.print("Pengiriman metode POST error: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

void sendParam(uint32_t pub) 
{

  
  String url = "http://" + server + ":" + port + "/postkey";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  String pub_key = String(pub);
  
  int iv_size = sizeof(iv);
  char encoded_iv[64];
  encode_base64(iv, iv_size, (unsigned char*)encoded_iv);
  String iv = String(encoded_iv);
  Serial.println(iv);
  int httpResponseCode = http.POST("{\"pub_key\":\"" + pub_key + "\", \"iv\":\"" + iv + "\"}");
  //int httpResponseCode = http.POST("{\"pub_key\":\"" + pub_key + "\"}");
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
  } else {
    Serial.print("Pengiriman metode POST error: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

void getParam() 
{
  String url = "http://" + server + ":" + port + "/pk";
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println(httpCode);
    Serial.println(payload);
    B = payload.toInt();
  } else {
    Serial.print("HTTP GET error ");
    Serial.println(httpCode);
  }
  http.end();
}

void intro() 
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(26, 20);
  display.println("Telemedicine");
  display.setCursor(15, 30);
  display.println("Body Temperature");
  display.display();
  delay(5000);
}

void tampilSuhu() 
{
  display.clearDisplay();
  display.setCursor(0, 20);
  display.print("Suhu Tubuh: ");
  display.print(objectT);
  display.print(" *C");
  display.setCursor(0, 40);
  display.print("Ambient: ");
  display.print(ambientT);
  display.print(" *C");
  display.display();
}

void makeParam() 
{
  RNG::fill(iv, 16);
  //memulai pertukaran kunci dengan diffie hellman
  a = keyGen();
  A = pow_mod(generator, a, prime);
  Serial.print("Shared index is: ");
  Serial.println(A);

  getParam();

  Serial.print("Received shared index is: ");
  Serial.println(B);

  Ka = pow_mod(B, a, prime);

  Serial.print("Shared Key A: ");
  Serial.println(Ka);

  char buffer[16];
  char *payload = buffer;
  sprintf(buffer, "%lu", Ka);

  hashing(payload);
  
  Serial.print("Hash: ");
  for (byte i = 0; i < 16; i++) {
      if (shaResult[i]<0x10) { Serial.print('0'); }
      Serial.print(shaResult[i], HEX);
  }
  Serial.print("\n");
  Serial.println("");
}

void setup() {
  Serial.begin(115200);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  intro();  
  WiFi.mode(WIFI_STA);

  wifi_connect();

  makeParam();
  
  temp.begin();
  temp.setUnit(TEMP_C);

  Serial.println("send parameter");
  sendParam(A);
}

void loop() {
  char buf[16];
  
  if (temp.read()) {
    objectT = temp.object();
    ambientT = temp.ambient();
  }

  char *plain_text = gcvt(objectT, 4, buf);

  tampilSuhu();

  Serial.print("Proses ke: ");
  Serial.println(count);


  // encrypt
  int length = 0;
  bufferSize(plain_text, length);
  char encrypted[length];

  unsigned long start = micros();
  encrypt(plain_text, encrypted, length);
  unsigned long end = micros() - start;
  Serial.print("SHA128 Encryption time: ");
  Serial.print(end);
  Serial.println(" us");
  
  Serial.println("");
  Serial.print("Encrypted: ");
  Serial.println(encrypted);


  postData(encrypted);

  // decrypt
  length = strlen(encrypted);
  char decrypted[length];
  decrypt(encrypted, decrypted, length);

  Serial.print("Decrypted: ");
  Serial.println(decrypted);

  int i;
  Serial.print("key: ");
  for ( i = 0; i < BLOCK_SIZE; i++ ) {
    printf( "%02X", shaResult[i] );
  }
  printf( "\n" );

  Serial.print("iv: ");
  for ( i = 0; i < 16; i++ ) {
    printf( "%02X", iv[i] );
  }
  printf( "\n" );
  printf( "\n" );

  count++;
  delay(500);
}