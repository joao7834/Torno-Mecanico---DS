#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>

// OLED
const int SC_WD = 128; // Largura
const int SC_HG = 64;  // Altura
const int OL_RST = -1; // Reset
Adafruit_SSD1306 display(SC_WD, SC_HG, &Wire, OL_RST);

// Pinos
const int P_POT = 34;
const int P_DHT = 15; 
const int P_RLY = 18; 
const int P_SDA = 21;
const int P_SCL = 22;

// DHT
const int DHT_TYPE = DHT22;
DHT dht(P_DHT, DHT_TYPE);

// BMP
Adafruit_BMP085 bmp;
const float SEA_PRESSION = 101325.0; // Pressão padrão em Pa

const float R_TNS = 127.0; // Tensão residencial simulada
const float M_COR = 10.0;  // Corrente máxima 
const float L_COR = 7.0;   // Limite de proteção pra desligar o relay

// Controle de tempo para leitura do DHT
unsigned long lastDHTRead = 0;
float um = 0.0, tm = 0.0;

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("Iniciando ESP32...");

    pinMode(P_RLY, OUTPUT);
    digitalWrite(P_RLY, LOW); 

    // ADC ESP32
    analogReadResolution(12);
    pinMode(P_POT, INPUT);

    // I2C
    Wire.begin(P_SDA, P_SCL);

    // DHT22
    dht.begin();

    // OLED I2C
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("Falha ao iniciar o Display OLED"));
        for (;;);
    }

    // BMP085 / BMP180
    if (!bmp.begin()) {
        Serial.println(F("Falha ao encontrar o sensor BMP085/BMP180!"));
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(10, 25);
    display.println("Sistema Iniciado");
    display.display();
    delay(1000);
}

void loop() {
    //ACS712
    int lt = analogRead(P_POT); 
    float cr = (lt / 4095.0) * M_COR; 
    float pt = R_TNS * cr; 

    //DHT22
    if (millis() - lastDHTRead > 2000) {
        lastDHTRead = millis();
        float newUm = dht.readHumidity();
        float newTm = dht.readTemperature();

        if (!isnan(newUm) && !isnan(newTm)) {
            um = newUm;
            tm = newTm;
        }
    }
    
    //BMP
    float press = bmp.readPressure() / 100.0; // Converte Pa pra hPa

    //Relay
    bool proAt = false; 
    if (cr > L_COR) {
        digitalWrite(P_RLY, HIGH);
        proAt = true;
    } else {
        digitalWrite(P_RLY, LOW); 
    }

    Serial.print("Temp: "); Serial.print(tm, 1); Serial.print("C | ");
    Serial.print("Umid: "); Serial.print(um, 1); Serial.print("% | ");
    Serial.print("Press: "); Serial.print(press, 1); Serial.print("hPa | ");
    Serial.print("Corr: "); Serial.print(cr, 2); Serial.print("A | ");
    Serial.print("Pot: "); Serial.print(pt, 1); Serial.print("W | ");
    Serial.println(proAt ? " [ALERTA: SOBRECARGA!]" : "[STATUS: OK]");

    display.clearDisplay();
    display.setTextSize(1);

    display.setCursor(0, 0);
    if (proAt) {
        display.println("!! SOBRECARGA AC !!");
    } else {
        display.println("MONITORAMENTO IOT");
    }
    display.drawLine(0, 9, 127, 9, SSD1306_WHITE);

    display.setCursor(0, 13);
    display.print("T: "); display.print(tm, 1); display.print(" C  | U: "); display.print(um, 0); display.println("%");

    display.setCursor(0, 25);
    display.print("P: "); display.print(press, 1); display.println(" hPa");

    display.setCursor(0, 37);
    display.print("Corr: "); display.print(cr, 2); display.println(" A");

    display.setCursor(0, 49);
    display.print("Pot: "); display.print(pt, 1); display.println(" W");    

    display.display();

    delay(200);
}