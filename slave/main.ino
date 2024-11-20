// Comunicacion XBee por Serial1 a Serial
// El modulo de comunicación XBee requiere que
// el conector de alimentación este conectado y
// que el display de bateria marque más de 5.0 voltios
#include <SPI.h>
#include <WiFi101.h>
#include "DHT.h"
#include <ArduinoHttpClient.h>

#define PIN_RED       6   // Pin led rojo
#define PIN_GREEN     3   // Pin led verde
#define PIN_BLUE      2   // Pin led azul  
#define PIN_BATERIA   A0  // Pin voltaje de la bateria

#define DHTTYPE DHT11     // Usamos el sensor DHT11
#define PIN_DHT 9         // Pin sensor de temperatura
// Constantes
const unsigned long postingInterval = 3 * 1000; // Intervalo mínmo para el envio de datos 2*1000 ms
const unsigned int sensorCount = 3;             // Numero de datos a enviar

DHT dht(PIN_DHT, DHTTYPE); // Se crea el objeto dht

// Variables para la conexión WiFi
char ssid[] = "Jhon";     // Network SSID (name)
char pass[] = "876543210";       // Network password
int status = WL_IDLE_STATUS;    // Network status

char xBeeData;
int blinkTime = 500;
int blinkTimeStamp;
int tmpCentinela = 0;
boolean isHigh = true;
float T = 0, H = 0, B = 0; //Var Environment Temperature (t) as float

// Variables
char* host = "isa.requestcatcher.com"; // No incluir el https://
char* path = "/post";                  // Path
int   port = 80;                       // Puerto

WiFiClient wifi;
HttpClient client = HttpClient(wifi, host, port);

String response;
int statusCode = 0;

void WiFiInit() {
  // Le damos tiempo al shield WiFi de iniciar:
  delay(1000);

  // Verificamos la presencia del shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("[WIFI]\tShield not present");
    while (true); // No continuamos
  }

  String fv = WiFi.firmwareVersion();
  if (fv <= "1.1.0" )
    Serial.println("[WIFI]\tPor favor actualizar el firmware");

  // Intentando conectarse al WiFi:
  while (status != WL_CONNECTED) {
    Serial.print("[WIFI]\tIntento de connexión al SSID: ");
    Serial.println(ssid);
    // Conexión a una red por WPA/WPA2.
    // Cambiar la siguiente linea si usa un WIFI abierto o red WEP:
    status = WiFi.begin(ssid, pass);

    // Esperando 1 segundo para conexión:
    delay(1000);
  }
  // Estamos conectados, mostramos la información de la conexión:
  printWifiStatus();
}


void POST_send(int sensorCount, char* sensorNames[], float values[]) {
  String contentType = "application/json";
  String postData = "";
  String request = String(path) + "?";
  for (int idx = 0; idx < sensorCount; idx++) {
    if (idx != 0) request += "&";
    request += sensorNames[idx];
    request += "=";
    request += values[idx];
  }
  client.post(request, contentType, postData);
  statusCode = client.responseStatusCode();
  response = client.responseBody();
  Serial.print("status-code: ");
  Serial.println(statusCode);
}

char* nameArray[] = {"temperature", "humidity", "battery"}; // Nombres de las variables en la nube
float sensorValues[sensorCount];        // Vector de valores
unsigned long lastConnectionTime = 0;   // Marca temporal para el ultimo envio de datos

void POST(float T, float H, float B) {
  sensorValues[0] = T;
  sensorValues[1] = H;
  sensorValues[2] = B;
  if ((millis() - lastConnectionTime) > postingInterval)
  {
    Serial.print("[SEND]\tEnviando datos > ");
    POST_send(sensorCount, nameArray, sensorValues);  // REST call
    lastConnectionTime = millis();  // Actualizamos la marca temporal
  }
}

void setup()
{ 
  Serial.begin(115200);
  Serial1.begin(9600);
  // Comunicación
  WiFiInit(); // Iniciando la conexión WIFI

  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  blinkTimeStamp = millis();
}

void loop()
{ 
  if (Serial1.available() > 0)
  {
    xBeeData = (char)Serial1.read();
    Serial.println(xBeeData);
    if (xBeeData == 'A'){
        isHigh = true;
         tmpCentinela = 0;
    }else if (xBeeData == 'O'){
      isHigh = false;
    }
  }

  if (isHigh){
    POST(T, H, B);   
    readSensors();
  }else{
    //No need to send anythin
    //POST("Temperature is OK");
    tmpCentinela = 2;
  }
checkStatus();
}

void readSensors(){
  T = dht.readTemperature();
  H = dht.readHumidity();
  int valEntrada = analogRead(PIN_BATERIA);  // Leemos la información de la bateria
  B = (float)valEntrada*9.2/1023.0;  // Convertimos la lectura en voltios
}

void checkStatus(){
  int currTime = millis();
  Serial.println(tmpCentinela);
  switch (tmpCentinela){
    case 0:
      digitalWrite(PIN_RED, HIGH);
      digitalWrite(PIN_GREEN, LOW);
      if (currTime - blinkTimeStamp > blinkTime){
        blinkTimeStamp = currTime;
        tmpCentinela = 1;
        Serial.println(tmpCentinela);
      }
      break;
    case 1:
      digitalWrite(PIN_RED, LOW);
      digitalWrite(PIN_GREEN, LOW);
      if (currTime - blinkTimeStamp > blinkTime){
        blinkTimeStamp = currTime;
        tmpCentinela = 0;
      }
      Serial.println(tmpCentinela);
    break;
    case 2:
      digitalWrite(PIN_GREEN, HIGH);
      break;
    default:
      
      digitalWrite(PIN_RED, LOW);
      digitalWrite(PIN_GREEN, LOW);
      break;
  }
}

void printWifiStatus() {
  // Muestra el nombre de la red WiFi
  Serial.print("[WIFI]\tConectado a SSID: ");
  Serial.println(WiFi.SSID());

  // Muestra la dirección IP
  IPAddress ip = WiFi.localIP();
  Serial.print("[WIFI]\tDirección IP: ");
  Serial.println(ip);

  // Muestra la intensidad de señal:
  long rssi = WiFi.RSSI();
  Serial.print("[WIFI]\tIntensidad de señal (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}

