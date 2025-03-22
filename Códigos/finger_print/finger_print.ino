#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

HardwareSerial mySerial(1);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
  Serial.begin(115200);
  mySerial.begin(57600, SERIAL_8N1, 16, 17);

  finger.begin(57600);
  if (!finger.verifyPassword()) {
    Serial.println("Erro: Sensor não encontrado!");
    while (1);
  }
}

void loop() {
  Serial.println("Posicione o dedo para autenticação...");
  int id = verificarDigital();

  if (id > 0) {
    Serial.print("Usuário autenticado! ID: ");
    Serial.println(id);
    
    // Aqui você pode acionar a leitura do ECG, abrir portas, etc.
  } else {
    Serial.println("Falha na autenticação!");
  }

  delay(2000);
}

int verificarDigital() {
  int resultado = finger.getImage();
  if (resultado != FINGERPRINT_OK) return -1;

  resultado = finger.image2Tz();
  if (resultado != FINGERPRINT_OK) return -1;

  resultado = finger.fingerFastSearch();
  if (resultado == FINGERPRINT_OK) {
    return finger.fingerID;  // Retorna o ID do usuário autenticado
  }

  return -1;  // Falha na autenticação
}