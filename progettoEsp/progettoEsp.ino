#include <gfxfont.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>

//Progetto Tirocinio


#include <Adafruit_Fingerprint.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define TIME_REQUEST  7     // ASCII bell character requests a time sync message 

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1


const unsigned char fingerprint_icon [] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x70, 0x70, 0x78, 0x78, 0x3C, 0x3C, 0x1E, 0x1E, 0x1E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
0x0F, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x1E, 0x1E, 0x3C, 0x3C, 0x78, 0x78, 0xF8, 0xF0,
0xE0, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 
};




SoftwareSerial mySerial1(3, 2);
SoftwareSerial mySerial2(13, 15);

Adafruit_Fingerprint finger1 = Adafruit_Fingerprint(&mySerial1);
Adafruit_Fingerprint finger2 = Adafruit_Fingerprint(&mySerial2);

//DISPLAY
Adafruit_SSD1306 display1(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SSD1306 display2(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//WIFI
char* ssid[] = {"Telefono Mi", "FRITZ!Box2.4","EXTENDER", "ASUS"};
const char* password[] = {"045e197f3f12", "53304164426232275192", "53304164426232275192", "Mariolina"};

//MQTT
const char* mqtt_server = "51.145.149.235";


char id[10];
char buf[50];

//WIFI
WiFiClient espClient;

//MQTT
PubSubClient client(espClient);

//TIME
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;
// Offset time
int GTMoffset = 2;


void setup()
{
  Serial.begin(9600);
  Serial.println();
  Serial.println();
  
  //Begin display
  display1.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  display2.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  display1.clearDisplay();  //Pulisce il buffer da inviare al display
  display2.clearDisplay();  //Pulisce il buffer da inviare al display
  
  //Begin MQTT
  client.setServer(mqtt_server, 1883);
  setup_wifi();
  
  
  Serial.println("\n\nAdafruit finger detect test");
  
  // Initialize a NTPClient to get time
  timeClient.begin();
  timeClient.setTimeOffset(GTMoffset * 3600);
  
  // set the data rate for the sensor serial port
  finger1.begin(57600);
  finger2.begin(57600);
  delay(5);
  init_finger();
  client.subscribe("/dev/esp8266/reg");
  client.subscribe("/dev/esp8266/del");
  client.subscribe("/dev/esp8266/reset");
  client.setCallback(callback);
  Serial.println("Waiting for finger");
  
}

void init_finger() {
  if (finger1.verifyPassword()) {
    Serial.println("Found 1st fingerprint sensor!");
  } else {
    Serial.println("Did not find 1st fingerprint sensor :(");
    while (1) {
      delay(1);
    }
  }
  if (finger2.verifyPassword()) {
    Serial.println("Found 2nd fingerprint sensor!");
  } else {
    Serial.println("Did not find 2nd fingerprint sensor :(");
    while (1) {
      delay(1);
    }
  }

  
  finger1.getTemplateCount();
  finger2.getTemplateCount();

  if (finger1.templateCount == 0) {
    Serial.println("Sensor 1 doesn't contain any fingerprint data.");
  }
  else {
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor 1 contains "); Serial.print(finger1.templateCount); Serial.println(" templates");
    Serial.println();
  }

  if (finger2.templateCount == 0) {
    Serial.println("Sensor 2 doesn't contain any fingerprint data.");
  }
  else {
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor 2 contains "); Serial.print(finger2.templateCount); Serial.println(" templates");
    Serial.println();
  }
}

void setup_wifi() {
  int count=0;
  int timeout=0;
  bool incr=true;
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  display1.clearDisplay();
  display2.clearDisplay();
  
  stampa(&display1,"Provo a connettermi",0,1.2);
  stampa(&display1,"al WIFI...",15,1.2);
  
  stampa(&display2,"Provo a connettermi",0,1.2);
  stampa(&display2,"al WIFI...",15,1.2);
  

  
  
  while (WiFi.status() != WL_CONNECTED){
    Serial.print("Provo a connettermi alla rete: ");
    Serial.println(ssid[count]);
    WiFi.begin(ssid[count], password[count]);
    while (WiFi.status() != WL_CONNECTED && timeout < 50) {
      delay(200);
      timeout++;
      Serial.print(".");
    }
    Serial.println();
    timeout=0;
    
    if((count < 2 && incr == true) || (count  == 0)){
        count++;
        incr=true;
    }else{
        count--;
        incr=false;
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  display1.clearDisplay();
  display2.clearDisplay();

  stampa(&display1,"CONNESSO!",0,1.3);
  stampa(&display2,"CONNESSO!",0,1.3);
  
  //MQTT Connection
  delay(3000);
  reconnect();
  delay(3000);
}

void callback(char* topic, byte* message, unsigned int length) {
  char buffer[128];
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  // Form a C-string from the payload
  memcpy(buffer, message, length);
  buffer[length] = '\0';

  // Convert it to integer
  char *end = nullptr;
  int value = strtol(buffer, &end, 10);
  Serial.print("Valore value: "); Serial.println(value);
  if(strcmp(topic, "/dev/esp8266/reg") == 0){
    getFingerprintEnroll(&finger1,value);
    getFingerprintEnroll(&finger2,value);
  }
  if(strcmp(topic, "/dev/esp8266/del") == 0){
    delFingerprint(&finger1,value);
    delFingerprint(&finger2,value);
  }
  if(strcmp(topic, "/dev/esp8266/reset") == 0){
    if(value == 1){
      Serial.println("Reset");
      finger1.emptyDatabase();
      finger2.emptyDatabase();
      display1.clearDisplay();
      display2.clearDisplay(); 
      stampa(&display1,"RESET DATABASE",0,1.2);
      stampa(&display2,"RESET DATABASE",0,1.2);
    }
  }
  delay(5000);

}

void reconnect() {
  // Loop until we're reconnected
  stampa(&display1,"CONNESSIONE MQTT...",15,1.2);
  stampa(&display2,"CONNESSIONE MQTT...",15,1.2);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (client.connect("espClient")) {
      Serial.println("connected");
      Serial.print(client.state());
      // Subscribe
      client.subscribe("/dev/esp8266/reg");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


// returns -1 if failed, otherwise returns ID #
int getFingerprintID() {
  bool isFinger1 = false; 
  bool isFinger2 = false; 
    
  uint8_t p1 = finger1.getImage();
  uint8_t p2 = finger2.getImage();

// Check finger
  if (p1 == FINGERPRINT_OK)  isFinger1 = true;
  if (p2 == FINGERPRINT_OK)  isFinger2 = true;

  if (isFinger1){
    Serial.println("Finger 1 detected");
    p1 = finger1.image2Tz();
    if (p1 != FINGERPRINT_OK)  return -1;

    p1 = finger1.fingerFastSearch();
    if (p1 != FINGERPRINT_OK)  return -1;

    
    // found a match!
    Serial.print("Found ID #"); Serial.print(finger1.fingerID);
    Serial.print(" with confidence of "); Serial.println(finger1.confidence);
    Serial.println(finger1.fingerID);
    itoa(finger1.fingerID,id,10);
    Serial.println("conversione");
    getTime(id,isFinger1,isFinger2);
    
    //Invio i dati al server
    if (buf != "-1"){
      client.publish("/dev/esp8266/accessi",buf);
      Serial.println("Sent to server");
      Serial.println(buf);
      display1.clearDisplay();
      display2.clearDisplay();
      stampa(&display1,"Accesso consentito",0,1.2);
      stampa(&display2,"Accesso consentito",0,1.2);
      stampa(&display1,"ID Dipendente: "+ finger2.fingerID,15,1.2);
      stampa(&display2,"ID Dipendente: " + finger2.fingerID,15,1.2);
      delay(2000);
    } else {
      return -1;
    }
    Serial.println("Remove Finger");
    return finger1.fingerID;
  }

  if (isFinger2){
    Serial.println("Finger 2 detected");
    p2 = finger2.image2Tz();
    if (p2 != FINGERPRINT_OK)  return -1;

    p2 = finger2.fingerFastSearch();
    if (p2 != FINGERPRINT_OK)  return -1;
    
    // found a match!
    Serial.print("Found ID #"); Serial.print(finger2.fingerID);
    Serial.print(" with confidence of "); Serial.println(finger2.confidence);
    itoa(finger2.fingerID,id,10);
    getTime(id,isFinger1,isFinger2);
    
    //Invio i dati al server
    if (buf != "-1"){
      client.publish("/dev/esp8266/accessi",buf);
      Serial.println("Sent to server");
      Serial.println(buf);
      display1.clearDisplay();
      display2.clearDisplay();
      stampa(&display1,"Uscita consentita",0,1.2);
      stampa(&display2,"Uscita consentita",0,1.2);
      stampa(&display1,"ID Dipendente: "+ finger2.fingerID,15,1.2);
      stampa(&display2,"ID Dipendente: " + finger2.fingerID,15,1.2);
    } else {
      return -1;
    }

    Serial.println("Remove Finger");
    delay(2000);
    return finger2.fingerID;
  }
}


uint8_t getFingerprintEnroll(Adafruit_Fingerprint *finger,int id) {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  display1.clearDisplay();
  display2.clearDisplay();
  stampa(&display1,"Registrare Dipendente:",0,1.2);
  stampa(&display1,String(id,DEC),15,1.3);
  stampa(&display2,"Registrare Dipendente:",0,1.2);
  stampa(&display2,String(id,DEC),15,1.3);

  while (p != FINGERPRINT_OK) {
    p = finger->getImage();
  }

  // OK success!

  p = finger->image2Tz(1);
  if (p != FINGERPRINT_OK)  return p;
  
  Serial.println("Remove finger");
  display1.clearDisplay();
  display2.clearDisplay();
  stampa(&display1,"Togliere il dito",0,1.2);
  stampa(&display2,"Togliere il dito",0,1.2);

  delay(2000);
  
  p = 0;
  
  while (p != FINGERPRINT_NOFINGER) {
    p = finger->getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  display1.clearDisplay();
  display2.clearDisplay();
  stampa(&display1,"Avvicinare lo stesso",0,1.2);
  stampa(&display2,"Avvicinare lo stesso",0,1.2);
  stampa(&display1,"dito",15,1.2);
  stampa(&display2,"dito",15,1.2);
  while (p != FINGERPRINT_OK) {
    p = finger->getImage();
  }

  // OK success!

  p = finger->image2Tz(2);
  if (p != FINGERPRINT_OK)  return p;
  

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger->createModel();
  
  if (p != FINGERPRINT_OK)  return p;
    
  Serial.print("ID "); Serial.println(id);
  
  p = finger->storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    display1.clearDisplay();
    display2.clearDisplay();
    stampa(&display1,"ARCHIVIATO!",0,1.2);
    stampa(&display2,"ARCHIVIATO!",0,1.2);
  } else {
    Serial.println("ERROR!");
    return p;
  }

  return true;
}



char* getTime(char* id, bool isFinger1, bool isFinger2){
  
  char timeChar[20];
  char dateChar[20];
  
  // Get date and time
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
   formattedDate = timeClient.getFormattedDate();

  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);

  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);

  dayStamp.toCharArray(dateChar,20);
  timeStamp.toCharArray(timeChar,20);
  
  //Creo una stringa con id;orario;data da inviare tramite MQTT al server
  if(isFinger1){
    strcpy(buf,"e");strcat(buf,";");
    strcat(buf,id);strcat(buf,";");
    strcat(buf,dateChar);strcat(buf,";");
    strcat(buf,timeChar);
  } else if(isFinger2){
    strcpy(buf,"u");strcat(buf,";");
    strcat(buf,id);strcat(buf,";");
    strcat(buf,dateChar);strcat(buf,";");
    strcat(buf,timeChar);
    }
}

uint8_t delFingerprint(Adafruit_Fingerprint *finger,int id) {
  uint8_t p = -1;
  p = finger->deleteModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
  }
}

void loop()                     
{
  client.loop();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Disconnesso");
    while (WiFi.status() != WL_CONNECTED) {
      setup_wifi();
    }
  }

  if (!client.connected()) {
    reconnect();
  }

  display1.clearDisplay();
  display2.clearDisplay();
  stampa(&display1,"ENTRATA!",0,1.2);
  stampa(&display1,"Waiting for finger...",15,1.2);
  stampa(&display2,"USCITA!",0,1.2);
  stampa(&display2,"Waiting for finger...",15,1.2);
  /*display1.clearDisplay();
  display1.drawBitmap(40, 20,  fingerprint_icon, 60, 20, 1);*/
  display1.display();
  getFingerprintID();
  
  delay(50);
}


void stampa(Adafruit_SSD1306 *display,String stringa, int riga,float size){
  
  display->setTextSize(size);  //Imposta la grandezza del testo
  display->setTextColor(WHITE); //Imposta il colore del testo (Solo bianco)
  display->setCursor(0,riga); //Imposta la posizione del cursore (Larghezza,Altezza)
  display->println(stringa); //Stringa da visualizzare
  display->display(); //Invia il buffer da visualizzare al display


}