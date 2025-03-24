//     URL: https://github.com/RobTillaart/Vibration


#include "Vibration.h"
#include "WiFiS3.h"

//system states
#define NORMAL    0
#define WARNING   1
#define EMERGENCY 2

// tilt sensor states
#define UP        3
#define DOWN      4

// tilt pin
#define TILT      5

//light pin
#define GREEN     6
#define YELLOW    7
#define RED       8

//activity threshold
#define IDLE    100
#define UNUSUAL 250

//modify ssid & pass (consider .env file)
char ssid[] = "";   
char pass[] = "";  
int keyIndex = 0; 

int status = WL_IDLE_STATUS;

VibrationSensor VBS(A0);

WiFiClient client;
//char server[] = "example.org";
IPAddress server(192,168,1,101);

uint32_t previous_avr;
uint32_t current_avr;
uint32_t unusual_avr;
int counter; //counts until emergency
int counter2; //counts until back to normal
int user_state; //for normal/warning/emergency

void setup()
{
  
  Serial.begin(115200);
  Serial.print("Startup");
  Serial.println();

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

  }
  printWifiStatus();

  pinMode(TILT, INPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(YELLOW, OUTPUT); 
  pinMode(RED, OUTPUT); 

  previous_avr = 0;
  current_avr = 0;
  unusual_avr = 0;
  counter = 0;
  counter2 = 0;
  user_state = NORMAL;
}


void loop()
{
  measure_vibration();
  check_user_activity();
  read_request();
  httpRequest();
}

void check_user_activity() {
    /**
  Resting value: typically < 100
  Walking value: typically 100 - 250

  Unusual vibration value should be approx 250+ (assume bodyweight impact w/ surface)
  check also user orientation (did they switch from vertical-horizontal or vice versa?)
  */

  switch(user_state) {
    case NORMAL:
      digitalWrite(GREEN, HIGH);
      //detect unusual vibration
      if (current_avr >= UNUSUAL) {
        Serial.print("Warning! Unusual vibration detected \n");
        user_state = WARNING;
        unusual_avr = current_avr;
      }
      break;
    case WARNING:
      digitalWrite(GREEN, LOW);
      digitalWrite(YELLOW, HIGH);
      //check if user is also horizontal
      if (current_avr <= IDLE && current_avr == previous_avr && counter < 5 && digitalRead(TILT) == LOW) {
          Serial.print("The user may be unconscious... ");
          Serial.print(counter);
          Serial.print(" seconds elapsed before contacting emergency");
          counter++;
      }
      else {
        counter2++;
      }
      //check if 5 counts of inactivity after impact has elapsed before notifying
      if (counter == 5) {
        user_state = EMERGENCY;
      }
      else if (counter2 == 10) { //assume activity resumed so no fainting
        digitalWrite(YELLOW, LOW);
        Serial.print("No problem. Back to normal \n");
        counter = 0;
        counter2 = 0;
        user_state = NORMAL;
      }  
      break;
    case EMERGENCY:
      digitalWrite(YELLOW, LOW);
      digitalWrite(RED, HIGH);
      Serial.print("HELP!! EMERGENCY!! \n");
      break;
  }
  previous_avr = current_avr;
}

void measure_vibration() {
//  measure for one second
  VBS.measure(1000000);
  //  average with one decimal
  Serial.print("Samples: \t");
  Serial.print(VBS.sampleCount());
  Serial.print("\t avg: \t");
  current_avr = VBS.average();
  Serial.print(current_avr);
  Serial.println();
}

//read req from server (TODO)
void read_request() {  
  uint32_t received_data_num = 0;

  while (client.available()) {
    /* actual data reception */
    char c = client.read();
    /* print data to serial port */
    Serial.print(c);
    /* wrap data to 80 columns*/
    received_data_num++;
    if(received_data_num % 80 == 0) { 
      
    }
    
  }  
}

void httpRequest() {
  std::string msg;
  switch(user_state) {
    case NORMAL: msg = "NORMAL"; break;
    case WARNING: msg = "WARNING"; break;
    case EMERGENCY: msg = "EMERGENCY"; break;
  }

  if (client.connect(server, 8000)) {  // Make sure 'server' has your computer's IP address
    Serial.println("Sending msg to local server");
    client.print("GET /submit?status=");
    client.print(msg.c_str());
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(server);  // Add this line to set the correct Host header
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("connection failed");
  }
}

void printWifiStatus() {  
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}