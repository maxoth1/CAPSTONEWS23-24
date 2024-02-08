

 Arduino Waage mit WiFi-Anbindung

Diese Anleitung führt Sie durch die Schritte, um Ihre Arduino Waage mit WiFi-Anbindung einzurichten und zu verwenden. Die Waage nutzt den HX711 ADC für präzise Gewichtsmessungen und verbindet sich über WiFi mit Ihrem Netzwerk, um Messdaten über einen Webbrowser zugänglich zu machen.

## Erste Schritte

Folgen Sie diesen Schritten, um Ihr Projekt einzurichten:

### 1. Vorbereitung der Arduino IDE

Stellen Sie sicher, dass Sie die neueste Version der Arduino IDE installiert haben. Sie können die Arduino IDE von [https://www.arduino.cc/en/Main/Software](https://www.arduino.cc/en/Main/Software) herunterladen.

### 2. Code aus GitHub laden

Klonen oder laden Sie den Projektcode von GitHub herunter. Öffnen Sie die `.ino`-Datei mit der Arduino IDE.

### 3. Bibliotheken installieren

Für dieses Projekt benötigen Sie zwei Bibliotheken:

- **HX711_ADC** von Olav Kallhovd, für die Waage.
- **Arduino Uno WiFi Dev Ed Library** von Arduino, für die WiFi-Verbindung.

So installieren Sie diese Bibliotheken in der Arduino IDE:

- Gehen Sie zu **Werkzeuge** > **Bibliothek verwalten...**
- Suchen Sie nach `HX711_ADC` und installieren Sie die Bibliothek.
- Suchen Sie nach `Arduino Uno WiFi Dev Ed Library` und installieren Sie auch diese Bibliothek.

### 4. WiFi-Konfiguration

Ändern Sie die folgenden Zeilen im Code, um sie mit Ihren WiFi-Daten zu konfigurieren:

```cpp
const char* ssid = "WLAN-NAME";
const char* password = "WLAN-PASSWORT";
```

Ersetzen Sie `WLAN-NAME` und `WLAN-PASSWORT` mit dem Namen (SSID) und Passwort Ihres WiFi-Netzwerks.

### 5. Code auf den Arduino hochladen

- Verbinden Sie Ihren Arduino über ein USB-Kabel mit Ihrem Computer.
- Wählen Sie das richtige Board und den Port in der Arduino IDE aus.
- Klicken Sie auf den **Hochladen**-Knopf, um den Code auf Ihren Arduino zu übertragen.

### 6. Verbindung überprüfen

- Öffnen Sie den Serial Monitor in der Arduino IDE und stellen Sie die Baudrate auf 115200 ein.
- Warten Sie, bis der Arduino sich erfolgreich mit Ihrem WiFi-Netzwerk verbindet. Die IP-Adresse des Arduino wird im Serial Monitor angezeigt.
- Geben Sie diese IP-Adresse in die Adresszeile Ihres Web-Browsers ein, um auf die Waage zuzugreifen.

Jetzt ist Ihr Arduino bereit, auch ohne direkte Verbindung zum PC zu laufen. Sie können ihn an eine Powerbank oder ein anderes USB-Netzteil anschließen.

