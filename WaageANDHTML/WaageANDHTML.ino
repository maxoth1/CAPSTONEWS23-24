/*
   -------------------------------------------------------------------------------------
   HX711_ADC
   Arduino library for HX711 24-Bit Analog-to-Digital Converter for Weight Scales
   Olav Kallhovd sept2017
   -------------------------------------------------------------------------------------
*/

/*
   This example file shows how to calibrate the load cell and optionally store the calibration
   value in EEPROM, and also how to change the value manually.
   The result value can then later be included in your project sketch or fetched from EEPROM.

   To implement calibration in your project sketch the simplified procedure is as follow:
       LoadCell.tare();
       //place known mass
       LoadCell.refreshDataSet();
       float newCalibrationValue = LoadCell.getNewCalibration(known_mass);
*/

#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

//pins:
const int HX711_dout = 4; //mcu > HX711 dout pin
const int HX711_sck = 5; //mcu > HX711 sck pin

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_eepromAdress = 0;
unsigned long t = 0;

#include <WiFi.h>

// WiFi-Einstellungen
const char* ssid = "Ija";
const char* password = "12345678";


// WiFiServer-Instanz:
WiFiServer server(80);

// Variable zum Speichern der HTTP-Anfrage:
String header;


// Function prototypes (inform the compiler about these functions before they are defined)
void calibrate();
void changeSavedCalFactor();


void setup() {
    Serial.begin(115200); 
    delay(10);

    // Verbinden mit dem WLAN
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: " + WiFi.localIP().toString());
    server.begin(); // Initialisieren Sie den Server nach erfolgreicher WiFi-Verbindung

    // Lade Kalibrierungswerte aus dem EEPROM
    float calValue;
    EEPROM.get(calVal_eepromAdress, calValue);
    if (calValue > 0) { // Einfache Validierung
        LoadCell.setCalFactor(calValue);
        Serial.println("Calibration value loaded from EEPROM.");
    } else {
        // Setze den Kalibrierungsfaktor auf einen Standardwert, falls keiner im EEPROM gespeichert ist
        LoadCell.setCalFactor(1.0);
        Serial.println("Default calibration factor set.");
    }
  

    // HX711 Setup
    Serial.println("Starting HX711...");
    LoadCell.begin();
    unsigned long stabilizingtime = 2000; // Stabilisierungszeit
    boolean _tare = true; // Tare bei Start
    LoadCell.start(stabilizingtime, _tare);
    while (!LoadCell.update());

    // Keine Notwendigkeit, calValue hier erneut zu deklarieren

    if (calValue > 0) { // Einfache Validierung
        LoadCell.setCalFactor(calValue);
    } else {
        // Setze den Kalibrierungsfaktor auf einen Standardwert, falls keiner im EEPROM gespeichert ist
        LoadCell.setCalFactor(-90.00); // Setzen eines Standard-Kalibrierungswertes
    }

    // Automatische Anpassung des Kalibrierungsfaktors, wenn das Gewicht außerhalb des gewünschten Bereichs liegt
    float weight = LoadCell.getData();
    if (weight < -0.9 || weight > 0.9) {
        float newCalibrationValue = -90.00; // Standard-Kalibrierungswert
        // Hier kannst du eine Logik einfügen, um newCalibrationValue basierend auf dem aktuellen Gewicht anzupassen
        LoadCell.setCalFactor(newCalibrationValue);
        EEPROM.put(calVal_eepromAdress, newCalibrationValue);
        EEPROM.commit();
        LoadCell.tareNoDelay(); // Tare nach der Einstellung des Kalibrierungsfaktors
        Serial.print("New calibration value set and tared: ");
        Serial.println(newCalibrationValue);
    }

    Serial.println("Startup is complete.");
}



void loop() {
  static boolean newDataReady = 0;
  const unsigned long serialPrintInterval = 5000; // Serial Druckintervall

  // HX711-Aktualisierung
  if (LoadCell.update()) newDataReady = true;

  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float i = LoadCell.getData();
      Serial.print("Load_cell output val: ");
      Serial.println(i);
      newDataReady = 0;
      t = millis();
    }
  } // Hier fehlte eine schließende Klammer

  // Lesen Sie das aktuelle Gewicht
  if (LoadCell.update()) {
    float currentWeight = LoadCell.getData();
    Serial.print("Aktuelles Gewicht: ");
    Serial.println(currentWeight);
  } // Schließende Klammer für den aktuellen Gewichtsaktualisierungsblock

  // Serieller Befehlsempfang
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') {
      LoadCell.tareNoDelay(); // Tare
    } else if (inByte == 'r') {
      calibrate(); // Kalibrieren
    } else if (inByte == 'c') {
      changeSavedCalFactor(); // Kalibrierungswert manuell ändern
    }
  }

  // Tare-Statusprüfung
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }

// Innerhalb von loop(), Verarbeitung der POST-Anfrage
WiFiClient client = server.available();
if (client) {
    String currentLine = "";
    while (client.connected()) {
        if (client.available()) {
            char c = client.read();
            header += c;
            if (c == '\n') {
                if (currentLine.length() == 0) {
                    // Verarbeiten der POST-Daten
                    if (header.indexOf("POST") != -1) {
                        // Warten, bis alle Daten empfangen wurden
                        delay(10);
                        while (client.available()) {
                            char c = client.read();
                            header += c;
                        }

                        // Tare-Anfrage verarbeiten
                        if (header.indexOf("tare=1") != -1) {
                            LoadCell.tareNoDelay();
                            Serial.println("Tare command received");
                        }

                        // Kalibrierungsanfrage verarbeiten
                        if (header.indexOf("calibrate=1") != -1) {
                            int startPos = header.indexOf("known_mass=");
                            if (startPos > 0) {
                                int endPos = header.indexOf('&', startPos);
                                String knownMassStr = header.substring(startPos + String("known_mass=").length(), endPos);
                                float knownMass = knownMassStr.toFloat();
                                if (knownMass > 0) {
                                    LoadCell.refreshDataSet(); // Optional, falls benötigt
                                    float newCalibrationValue = LoadCell.getNewCalibration(knownMass);
                                    EEPROM.put(calVal_eepromAdress, newCalibrationValue);
                                    EEPROM.commit();
                                    LoadCell.setCalFactor(newCalibrationValue);
                                    Serial.print("New calibration value set: ");
                                    Serial.println(newCalibrationValue);
                                }
                            }
                        }
                    }

// HTTP-Antwort senden
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-type:text/html");
                    client.println("Connection: close");
                    client.println();
                    client.println("<!DOCTYPE html><html>");
                    client.println("<head>");
                    client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
                    client.println("<style>");
                    client.println("body {");
                    client.println("  text-align: center;");
                    client.println("  margin: 0;");
                    client.println("  padding: 0;");
                    client.println("  background: linear-gradient(to bottom, #b3d1ff 0%, #99c2ff 100%);");
                    client.println("}");
                    client.println("h1 { color: #0f0f0f; }");
                    client.println("form { margin-top: 50px; }");
                    client.println("</style>");
                    client.println("<meta http-equiv=\"refresh\" content=\"30\">");
                    client.println("</head>");
                    client.println("<body>");
                    client.println("<div class=\"logo\">");
                    client.println("<img src=\"https://i.imgur.com/R9u9dc0.png\" alt=\"Logo\">");
                    client.println("</div>");
                    client.println("<h1>Arduino Waage</h1>");
                    client.print("<p>Aktuelles Gewicht: ");
                    client.print(LoadCell.getData());
                    client.println(" g</p>");

                    // Formular für Tare und Kalibrierung
                    client.println("<form action=\"/\" method=\"POST\">");
                    client.println("Tare: <input type=\"submit\" value=\"Tare\" name=\"tare\"><br><br>");
                    client.println("Bekannte Masse (Kalibrierung): <input type=\"number\" step=\"any\" name=\"known_mass\"><br><br>");
                    client.println("<input type=\"submit\" value=\"Kalibrieren\" name=\"calibrate\">");
                    client.println("</form>");

                    // Script zur Verarbeitung der POST-Anfrage
                    client.println("<script>");
                    client.println("document.querySelector('form').onsubmit = function(event) {");
                    client.println("  event.preventDefault();");
                    client.println("  var formElements = document.querySelector('form').elements;");
                    client.println("  var tareValue = formElements['tare'].value;");
                    client.println("  var knownMass = formElements['known_mass'].value;");
                    client.println("  // Fügen Sie hier die Logik zur Verarbeitung von Tare und Kalibrierung hinzu");
                    client.println("  if(tareValue) {");
                    client.println("    // Verarbeiten der Tare-Anfrage");
                    client.println("  }");
                    client.println("  if(knownMass) {");
                    client.println("    // Verarbeiten der Kalibrierungsanfrage");
                    client.println("  }");
                    client.println("};");
                    client.println("</script>");

                    client.println("</body></html>");
                    break;
                }
                currentLine = "";
            } else if (c != '\r') {
                currentLine += c;
            }
        }
    }
    header = ""; // Header für den nächsten Durchlauf leeren
    client.stop(); // Verbindung schließen
}
}

void calibrate() {
  Serial.println("***");
  Serial.println("Start calibration:");
  Serial.println("Place the load cell an a level stable surface.");
  Serial.println("Remove any load applied to the load cell.");
  Serial.println("Send 't' from serial monitor to set the tare offset.");

  boolean _resume = false;
  while (_resume == false) {
    LoadCell.update();
    if (Serial.available() > 0) {
      if (Serial.available() > 0) {
        char inByte = Serial.read();
        if (inByte == 't') LoadCell.tareNoDelay();
      }
    }
    if (LoadCell.getTareStatus() == true) {
      Serial.println("Tare complete");
      _resume = true;
    }
  }

  Serial.println("Now, place your known mass on the loadcell.");
  Serial.println("Then send the weight of this mass (i.e. 100.0) from serial monitor.");

  float known_mass = 0;
  _resume = false;
  while (_resume == false) {
    LoadCell.update();
    if (Serial.available() > 0) {
      known_mass = Serial.parseFloat();
      if (known_mass != 0) {
        Serial.print("Known mass is: ");
        Serial.println(known_mass);
        _resume = true;
      }
    }
  }

  LoadCell.refreshDataSet(); //refresh the dataset to be sure that the known mass is measured correct
  float newCalibrationValue = LoadCell.getNewCalibration(known_mass); //get the new calibration value

  Serial.print("New calibration value has been set to: ");
  Serial.print(newCalibrationValue);
  Serial.println(", use this as calibration value (calFactor) in your project sketch.");
  Serial.print("Save this value to EEPROM adress ");
  Serial.print(calVal_eepromAdress);
  Serial.println("? y/n");

  _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
 #if defined(ESP8266)|| defined(ESP32)
        EEPROM.begin(512);
 #endif
        EEPROM.put(calVal_eepromAdress, newCalibrationValue);
 #if defined(ESP8266)|| defined(ESP32)
        EEPROM.commit();
 #endif
        EEPROM.get(calVal_eepromAdress, newCalibrationValue);
        Serial.print("Value ");
        Serial.print(newCalibrationValue);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(calVal_eepromAdress);
        _resume = true;

      }
      else if (inByte == 'n') {
        Serial.println("Value not saved to EEPROM");
        _resume = true;
      }
    }
  }
  

  Serial.println("End calibration");
  Serial.println("***");
  Serial.println("To re-calibrate, send 'r' from serial monitor.");
  Serial.println("For manual edit of the calibration value, send 'c' from serial monitor.");
  Serial.println("***");
}


void changeSavedCalFactor(){
  float oldCalibrationValue = LoadCell.getCalFactor();
  boolean _resume = false;
  Serial.println("***");
  Serial.print("Current value is: ");
  Serial.println(oldCalibrationValue);
  Serial.println("Now, send the new value from serial monitor, i.e. 696.0");
  float newCalibrationValue;
  while (_resume == false) {
    if (Serial.available() > 0) {
      newCalibrationValue = Serial.parseFloat();
      if (newCalibrationValue != 0) {
        Serial.print("New calibration value is: ");
        Serial.println(newCalibrationValue);
        LoadCell.setCalFactor(newCalibrationValue);
        _resume = true;
      }
    }
  }
  _resume = false;
  Serial.print("Save this value to EEPROM adress ");
  Serial.print(calVal_eepromAdress);
  Serial.println("? y/n");
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
 #if defined(ESP8266)|| defined(ESP32)
        EEPROM.begin(512);
 #endif
        EEPROM.put(calVal_eepromAdress, newCalibrationValue);
 #if defined(ESP8266)|| defined(ESP32)
        EEPROM.commit();
 #endif
        EEPROM.get(calVal_eepromAdress, newCalibrationValue);
        Serial.print("Value ");
        Serial.print(newCalibrationValue);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(calVal_eepromAdress);
        _resume = true;
      }
      else if (inByte == 'n') {
        Serial.println("Value not saved to EEPROM");
        _resume = true;
      }
    }
  }
  Serial.println("End change calibration value");
  Serial.println("***");
  }










