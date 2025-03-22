#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

// Definir pinos e variáveis
HardwareSerial mySerial(1);  // Usando UART1 no ESP32 (GPIO16, GPIO17)
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Configuração de Wi-Fi
const char* ssid = "Starlink_CIT";  // Substitua pelo seu SSID
const char* password = "Ufrr@2024Cit";  // Substitua pela sua senha Wi-Fi
String servidorURL = "http://192.168.1.186:5000/dados";  // URL do servidor

// Definir pino do ECG
int ecgPin = 34;
int valorECG = 0;

uint8_t id = 100;  // ID do usuário para cadastro (pode ser alterado)

bool usuarioAutenticado = false;  // Variável para verificar se o usuário foi autenticado

void setup() {
  // Inicializar Serial
  Serial.begin(115200);
  
  // Inicializar comunicação UART com o sensor de impressão digital
  mySerial.begin(57600, SERIAL_8N1, 16, 17);  // Comunicação UART com o sensor
  
  // Inicializar sensor de impressão digital
  finger.begin(57600);
  if (!finger.verifyPassword()) {
    Serial.println("Erro: Sensor de biometria não detectado!");
    while (1);  // Aguarda erro
  }
  
  // Conectar-se à rede Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi!");
  Serial.print("IP_Address: ");
  Serial.println(WiFi.localIP());
  
  // Inicializar o processo de cadastro de digital
  Serial.println("Posicione o dedo para cadastro...");
  cadastrarDigital(id);
}

void loop() {
  // Se o usuário não estiver autenticado, realizar a autenticação
  if (!usuarioAutenticado) {
    Serial.println("Posicione o dedo para autenticação...");
    int idAutenticado = verificarDigital();

    if (idAutenticado > 0) {
      Serial.print("Usuário autenticado! ID: ");
      Serial.println(idAutenticado);
      usuarioAutenticado = true;  // Marcar como autenticado

      // Iniciar o envio de dados do ECG
      enviarECG();
    } else {
      Serial.println("Falha na autenticação!");
    }
  }

  // Continuar a enviar os dados do ECG enquanto o usuário estiver autenticado
  if (usuarioAutenticado) {
    enviarECG();
  }

  delay(2000);  // Delay entre as tentativas de leitura e envio do ECG
}

// Função para autenticar digital
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

// Função para cadastrar digital
void cadastrarDigital(uint8_t id) {
  Serial.println("Aguardando posicionamento do dedo...");

  // Captura a primeira imagem da digital
  while (finger.getImage() != FINGERPRINT_OK);
  Serial.println("Dedo detectado, processando...");
  if (finger.image2Tz(1) != FINGERPRINT_OK) {
    Serial.println("Erro ao processar imagem");
    return;
  }

  // Solicita para remover o dedo e posicioná-lo novamente
  Serial.println("Remova o dedo e posicione novamente...");
  delay(2000);

  // Aguardar remoção do dedo
  while (finger.getImage() != FINGERPRINT_NOFINGER);
  while (finger.getImage() != FINGERPRINT_OK);

  Serial.println("Segunda leitura capturada!");
  if (finger.image2Tz(2) != FINGERPRINT_OK) {
    Serial.println("Erro na segunda leitura");
    return;
  }

  // Criar o modelo de impressão digital
  if (finger.createModel() != FINGERPRINT_OK) {
    Serial.println("Erro ao gerar modelo");
    return;
  }

  // Salvar o modelo de impressão digital no banco de dados
  if (finger.storeModel(id) != FINGERPRINT_OK) {
    Serial.println("Erro ao salvar no banco de dados");
    return;
  }

  Serial.println("Digital cadastrada com sucesso!");
}

// Função para enviar os dados do ECG
void enviarECG() {
  // Leitura do ECG
  valorECG = analogRead(ecgPin);
  Serial.print("Valor ECG: ");
  Serial.println(valorECG);

  // Criar JSON com os dados do ECG
  String jsonPayload = "{\"ecg\": " + String(valorECG) + "}";

  // Enviar dados para o servidor
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(servidorURL);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      Serial.println("Dados enviados com sucesso!");
    } else {
      Serial.println("Erro ao enviar dados: " + String(httpResponseCode));
    }
    http.end();
  }
}
