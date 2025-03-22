#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

HardwareSerial mySerial(1);  // Usando UART1 no ESP32 (GPIO16, GPIO17)
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id = 100;  // ID do usuário (pode ser alterado)

void setup() {
  Serial.begin(115200);
  mySerial.begin(57600, SERIAL_8N1, 16, 17);  // Comunicação UART com o sensor

  finger.begin(57600);
  if (!finger.verifyPassword()) {
    Serial.println("Erro: Sensor de biometria não detectado!");
    while (1);
  }

  Serial.println("Posicione o dedo para cadastro...");
  cadastrarDigital(id);
}

void loop() {}

void cadastrarDigital(uint8_t id) {
  Serial.println("Aguardando posicionamento do dedo...");

  while (finger.getImage() != FINGERPRINT_OK);

  Serial.println("Dedo detectado, processando...");
  if (finger.image2Tz(1) != FINGERPRINT_OK) {
    Serial.println("Erro ao processar imagem");
    return;
  }

  Serial.println("Remova o dedo e posicione novamente...");
  delay(2000);
  
  while (finger.getImage() != FINGERPRINT_NOFINGER);
  while (finger.getImage() != FINGERPRINT_OK);

  Serial.println("Segunda leitura capturada!");
  if (finger.image2Tz(2) != FINGERPRINT_OK) {
    Serial.println("Erro na segunda leitura");
    return;
  }

  if (finger.createModel() != FINGERPRINT_OK) {
    Serial.println("Erro ao gerar modelo");
    return;
  }

  if (finger.storeModel(id) != FINGERPRINT_OK) {
    Serial.println("Erro ao salvar no banco de dados");
    return;
  }

  Serial.println("Digital cadastrada com sucesso!");
}
