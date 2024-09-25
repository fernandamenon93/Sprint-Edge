/*
  Adicione os tópicos de assinatura um para cada tópico que o ESP32 usa:
  topic_on_off_led/#

  Experimento publicar no topico topic_on_off_led com a mensagem 1 e 0
  para ligar e desligar o LED"
*/

#include <PubSubClient.h>
#include <WiFi.h>

#define PIN_LED 15 // GPIO que está ligado o LED

/* Configura os tópicos do MQTT */
#define TOPIC_SUBSCRIBE_LED       "topic_on_off_led"

#define PUBLISH_DELAY 2000   // Atraso da publicação (2 segundos)

#define ID_MQTT "esp32_mqtt" // id mqtt (para identificação de sessão)

// IMPORTANTE: Este deve ser único no broker, ou seja, se um client MQTT
// tentar entrar com o mesmo id de outro já conectado ao broker,
// o broker irá fechar a conexão de um deles.

/* Variáveis, constantes e objetos globais */
const char *SSID = "Wokwi-GUEST"; // SSID / nome da rede WI-FI que deseja se conectar
const char *PASSWORD = "";        // Senha da rede WI-FI que deseja se conectar

// URL do broker MQTT que se deseja utilizar
const char *BROKER_MQTT = "broker.hivemq.com";

int BROKER_PORT = 1883; // Porta do Broker MQTT

unsigned long publishUpdate;

// Variáveis e objetos globais
WiFiClient espClient;         // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient

void initWiFi(void);
void initMQTT(void);
void callbackMQTT(char *topic, byte *payload, unsigned int length);
void reconnectMQTT(void);
void reconnectWiFi(void);
void checkWiFIAndMQTT(void);

/* Inicializa e conecta-se na rede WI-FI desejada */
void initWiFi(void)
{
  delay(10);
  Serial.println("------Conexao WI-FI------");
  Serial.print("Conectando-se na rede: ");
  Serial.println(SSID);
  Serial.println("Aguarde");

  reconnectWiFi();
}

/* Inicializa os parâmetros de conexão MQTT(endereço do broker, porta e seta
  função de callback) */
void initMQTT(void)
{
  MQTT.setServer(BROKER_MQTT, BROKER_PORT); // Informa qual broker e porta deve ser conectado
  MQTT.setCallback(callbackMQTT);           // Atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}

/* Função de callback  esta função é chamada toda vez que uma informação
   de um dos tópicos subescritos chega */
void callbackMQTT(char *topic, byte *payload, unsigned int length)
{
  String msg;

  // Obtem a string do payload recebido
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    msg += c;
  }

  Serial.printf("Chegou a seguinte string via MQTT: %s do topico: %s\n", msg, topic);

  /* Toma ação dependendo da string recebida */
  if (msg.equals("1")) {
    digitalWrite(PIN_LED, HIGH);
    Serial.println("LED ligado por comando MQTT");
  }

  if (msg.equals("0")) {
    digitalWrite(PIN_LED, LOW);
    Serial.println("LED desligado por comando MQTT");
  }
}

/* Reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
   em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito. */
void reconnectMQTT(void)
{
  while (!MQTT.connected()) {
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado com sucesso ao broker MQTT!");
      MQTT.subscribe(TOPIC_SUBSCRIBE_LED);
    } else {
      Serial.println("Falha ao reconectar no broker.");
      Serial.println("Nova tentativa de conexao em 2 segundos.");
      delay(2000);
    }
  }
}

/* Verifica o estado das conexões WiFI e ao broker MQTT.
  Em caso de desconexão (qualquer uma das duas), a conexão é refeita. */
void checkWiFIAndMQTT(void)
{
  if (!MQTT.connected())
    reconnectMQTT(); // se não há conexão com o Broker, a conexão é refeita

  reconnectWiFi(); // se não há conexão com o WiFI, a conexão é refeita
}

void reconnectWiFi(void)
{
  // se já está conectado a rede WI-FI, nada é feito.
  // Caso contrário, são efetuadas tentativas de conexão
  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso na rede ");
  Serial.print(SSID);
  Serial.println("IP obtido: ");
  Serial.println(WiFi.localIP());
}

void setup()
{
  Serial.begin(115200);

  // Configura o pino do LED como output e inicializa em nível baixo
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  // Inicializa a conexão Wi-Fi
  initWiFi();

  // Inicializa a conexão ao broker MQTT
  initMQTT();
}

void loop()
{
  /* Repete o ciclo após 2 segundos */
  if ((millis() - publishUpdate) >= PUBLISH_DELAY) {
    publishUpdate = millis();
    // Verifica o funcionamento das conexões WiFi e ao broker MQTT
    checkWiFIAndMQTT();

    // Keep-alive da comunicação com broker MQTT
    MQTT.loop();
  }
}
