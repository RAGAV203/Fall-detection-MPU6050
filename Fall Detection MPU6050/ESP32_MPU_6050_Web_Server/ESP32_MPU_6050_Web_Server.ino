#include <Wire.h>
#include <WiFi.h>
#include <MPU6050_light.h>
MPU6050 mpu(Wire);
#define LED 2
const int MPU_addr=0x68;  // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
float ax=0, ay=0, az=0, gx=0, gy=0, gz=0;
boolean fall = false; //stores if a fall has occurred
boolean trigger1=false; //stores if first trigger (lower threshold) has occurred
boolean trigger2=false; //stores if second trigger (upper threshold) has occurred
boolean trigger3=false; //stores if third trigger (orientation change) has occurred
byte trigger1count=0; //stores the counts past since trigger 1 was set true
byte trigger2count=0; //stores the counts past since trigger 2 was set true
byte trigger3count=0; //stores the counts past since trigger 3 was set true
int angleChange=0;
int falld=0;
int prevAcX[5] = {0, 0, 0, 0, 0};
int prevAcY[5] = {0, 0, 0, 0, 0};
int prevAcZ[5] = {0, 0, 0, 0, 0};
int prevGyX[5] = {0, 0, 0, 0, 0};
int prevGyY[5] = {0, 0, 0, 0, 0};
int prevGyZ[5] = {0, 0, 0, 0, 0};

WiFiServer server(80);

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;
String output26State = "off";
String output27State = "off";

// WiFi network info.
const char *ssid = "------";   // Enter your Wi-Fi Name
const char *pass = "*******"; // Enter your Wi-Fi Password
String header;


void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.println("Wrote to IMU");
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("."); // print ... till not connected
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  digitalWrite(LED, HIGH);
  delay(200);
  digitalWrite(LED, LOW);
}

void loop() {
  detect();
  sendSensorData();
  delay(500);
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    String currentLine = ""; // Storing the incoming data in the string
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("<html><meta http-equiv=\"refresh\" content=\"0.5\"><title>ESP32 WebServer</title></html>");
            client.println("<style>");
            client.println("body { background-color: #1f1f1f; color: #ffffff; }");
            client.println("h1 { text-align: center; color: #4287f5; }");
            client.println(".container { border: 1px solid #ffffff; padding: 10px; margin-bottom: 20px; }");
            client.println(".value { font-size: 150%; }");
            client.println(".graph-container { height: 300px; width: 100%; }");
            client.println("</style>");
            client.println("<body>");
            client.println("<nav class=\"navbar\">");
            client.println("<span class=\"navbar-brand\">ESP32 WebServer</span>");
            client.println("</nav>");
            client.println("<div class=\"container\">");
            client.println("<h1>Accelerometer Values</h1>");
            client.println("<div class=\"value\">");
            client.print("<p>AcX: ");
            client.print(AcX);
            client.print("</p>");
            client.print("<p>AcY: ");
            client.print(AcY);
            client.print("</p>");
            client.print("<p>AcZ: ");
            client.print(AcZ);
            client.print("</p>");
            client.println("</div>");
            client.println("<div class=\"graph-container\">");
            client.println("<canvas id=\"accelerometer-chart\"></canvas>");
            client.println("</div>");
            client.println("</div>");
            client.println("<div class=\"container\">");
            client.println("<h1>Gyroscope Values</h1>");
            client.println("<div class=\"value\">");
            client.print("<p>GyX: ");
            client.print(GyX);
            client.print("</p>");
            client.print("<p>GyY: ");
            client.print(GyY);
            client.print("</p>");
            client.print("<p>GyZ: ");
            client.print(GyZ);
            client.print("</p>");
            client.println("</div>");
            client.println("<div class=\"graph-container\">");
            client.println("<canvas id=\"gyroscope-chart\"></canvas>");
            client.println("</div>");
            client.println("</div>");
            if (fall) {
              client.println("<center><h1 style=\"color: red;\">FALL DETECTED!</h1></center>");
              delay(2000);
              fall = false;
              client.println("<script>alert('Fall Detected! HELP!!!');</script>");
            } else {
              client.println("<center><h1>No FALL DETECTED</h1></center>");
            }
            client.println("<script src=\"https://cdn.jsdelivr.net/npm/chart.js\"></script>");
            client.println("<script>");
             client.println("var accelerometerData = {");
            client.println("  labels: ['1', '2', '3', '4', '5'],");
            client.println("  datasets: [{");
            client.println("    label: 'Accelerometer Values',");
            client.println("    data: [" + String(prevAcX[0]) + ", " + String(prevAcX[1]) + ", " + String(prevAcX[2]) + ", " + String(prevAcX[3]) + ", " + String(prevAcX[4]) + "],");
            client.println("    backgroundColor: 'rgba(66, 135, 245, 0.2)',");
            client.println("    borderColor: 'rgba(66, 135, 245, 1)',");
            client.println("    borderWidth: 1");
            client.println("  }]");
            client.println("};");
            client.println("var accelerometerCtx = document.getElementById('accelerometer-chart').getContext('2d');");
            client.println("var accelerometerChart = new Chart(accelerometerCtx, {");
            client.println("  type: 'line',");
            client.println("  data: accelerometerData,");
            client.println("  options: {");
            client.println("    responsive: true,");
            client.println("    scales: {");
            client.println("      y: {");
            client.println("        beginAtZero: true");
            client.println("      }");
            client.println("    }");
            client.println("  }");
            client.println("});");
            client.println("var gyroscopeData = {");
            client.println("  labels: ['1', '2', '3', '4', '5'],");
            client.println("  datasets: [{");
            client.println("    label: 'Gyroscope Values',");
            client.println("    data: [" + String(prevGyX[0]) + ", " + String(prevGyX[1]) + ", " + String(prevGyX[2]) + ", " + String(prevGyX[3]) + ", " + String(prevGyX[4]) + "],");
            client.println("    backgroundColor: 'rgba(66, 135, 245, 0.2)',");
            client.println("    borderColor: 'rgba(66, 135, 245, 1)',");
            client.println("    borderWidth: 1");
            client.println("  }]");
            client.println("};");
            client.println("var gyroscopeCtx = document.getElementById('gyroscope-chart').getContext('2d');");
            client.println("var gyroscopeChart = new Chart(gyroscopeCtx, {");
            client.println("  type: 'line',");
            client.println("  data: gyroscopeData,");
            client.println("  options: {");
            client.println("    responsive: true,");
            client.println("    scales: {");
            client.println("      y: {");
            client.println("        beginAtZero: true");
            client.println("      }");
            client.println("    }");
            client.println("  }");
            client.println("});");
            client.println("</script>");
            client.println("</body>");
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  // Shift the previous values in the arrays
  for (int i = 4; i > 0; i--) {
    prevAcX[i] = prevAcX[i - 1];
    prevAcY[i] = prevAcY[i - 1];
    prevAcZ[i] = prevAcZ[i - 1];
    prevGyX[i] = prevGyX[i - 1];
    prevGyY[i] = prevGyY[i - 1];
    prevGyZ[i] = prevGyZ[i - 1];
  }
  
  // Update the first element of the arrays with the current values
  prevAcX[0] = AcX;
  prevAcY[0] = AcY;
  prevAcZ[0] = AcZ;
  prevGyX[0] = GyX;
  prevGyY[0] = GyY;
  prevGyZ[0] = GyZ;

}

void detect(){  
 mpu_read();
 ax = (AcX-2050)/16384.00;
 ay = (AcY-77)/16384.00;
 az = (AcZ-1947)/16384.00;
 gx = (GyX+270)/131.07;
 gy = (GyY-351)/131.07;
 gz = (GyZ+136)/131.07;
 // calculating Amplitute vactor for 3 axis
 float Raw_Amp = pow(pow(ax,2)+pow(ay,2)+pow(az,2),0.5);
 int Amp = Raw_Amp * 10;  // Mulitiplied by 10 bcz values are between 0 to 1
 Serial.println(Amp);
 if (Amp<=2 && trigger2==false){ //if AM breaks lower threshold (0.4g)
   trigger1=true;
   Serial.println("TRIGGER 1 ACTIVATED");
   }
 if (trigger1==true){
   trigger1count++;
   if (Amp>=12){ //if AM breaks upper threshold (3g)
     trigger2=true;
     Serial.println("TRIGGER 2 ACTIVATED");
     trigger1=false; trigger1count=0;
     }
 }
 if (trigger2==true){
   trigger2count++;
   angleChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5); Serial.println(angleChange);
   if (angleChange>=30 && angleChange<=400){ //if orientation changes by between 80-100 degrees
     trigger3=true; trigger2=false; trigger2count=0;
     Serial.println(angleChange);
     Serial.println("TRIGGER 3 ACTIVATED");
       }
   }
 if (trigger3==true){
    trigger3count++;
    if (trigger3count>=1){ 
       angleChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5);
       //delay(10);
       Serial.println(angleChange); 
       if ((angleChange>=0) && (angleChange<=10)){ //if orientation changes remains between 0-10 degrees
           fall=true; trigger3=false; trigger3count=0;
           Serial.println(angleChange);
             }
       else{ //user regained normal orientation
          trigger3count=0;
          Serial.println("TRIGGER 3 DEACTIVATED");
       }
     }
  }
 if (fall==true){ //in event of a fall detection
   Serial.println("FALL DETECTED");
   falld=1;
    digitalWrite(LED,HIGH);
    delay(500);
    digitalWrite(LED,LOW);
    delay(500);
    digitalWrite(LED,HIGH);
    delay(500);
    digitalWrite(LED,LOW);
    delay(500);
    digitalWrite(LED,HIGH);
    delay(500);
   }
 //if (trigger2count>=6){ //allow 0.5s for orientation change
  // trigger2=false; trigger2count=0;
   //Serial.println("TRIGGER 2 DECACTIVATED");
  // }
 //if (trigger1count>=6){ //allow 0.5s for AM to break upper threshold
   //trigger1=false; trigger1count=0;
   //Serial.println("TRIGGER 1 DECACTIVATED");
   //}
  delay(100);
}



void mpu_read(){
 Wire.beginTransmission(MPU_addr);
 Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
 Wire.endTransmission(false);
 Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
 AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
 AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
 AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
 Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
 GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
 GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
 GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

 }

 void sendSensorData() {
 

 // Prepare the data string
 String data = "{" +String(AcX) + "," + String(AcY) + "," + String(AcZ) + "," + String(GyX) + "," + String(GyY) + "," + String(GyZ)+ "}";
  Serial.print("Content-Length: ");
  Serial.print(data.length());
  Serial.print("\r\n\r\n");
  Serial.print(data);
  Serial.print("\r\n\r\n");
  Serial.println("Data sent");
}
