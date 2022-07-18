/* Programa para Artigo do Estagio UNINTER:
 * Monitor o volume na sua caixa d'água
 * Autor: Thiago Pessanha - RU: 1220215
 * Biblioteca da Placa: "esp8266 by ESP8266 Community versão 2.3.0"
 * Placa: "NodeMCU 1.0 (ESP-12E Module)"
 * Upload Speed: "115200"
 * CPU Frequency: "160MHz"
*/
//=====================================================================
// --- Inclusão de bibliotecas ---
#include <ESP8266WiFi.h> 
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
// ======================================================================
// --- Dados de Acesso do seu roteador ---
#define WLAN_SSID       "NET_2G" // Informação da SSID do seu roteador
#define WLAN_PASS       "a315saex"   // senha de acesso do seu roteador
// ======================================================================
// --- Dados de Acesso da Plataforma Adafruit IO ---
#define AIO_SERVER      "io.adafruit.com"     // manter fixo
#define AIO_SERVERPORT  1883                  // manter fixo
#define AIO_USERNAME    "ThiagoPh"        // sua informação
#define AIO_KEY         "aio_efNG11K1LJH9FpsqtMJidWgATnC0" // sua informação
// ======================================================================
// --- Mapeamento de Hardware ---
#define trigPin 5  //D1 - PINO DE TRIGGER PARA SENSOR ULTRASSONICO
#define echoPin 4  //D2 - PINO DE ECHO PARA SENSOR ULTRASSONICO
 
WiFiClient client; // cria uma classe WiFiClient com o ESP8266 para conexão com o servidor MQTT
 
// Configura a classe MQTT passando para o WiFi cliente e Servidor MQTT os detalhes do login
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
 
// O endereçamento para enviar os arquivos seguem o formato: <username>/feeds/<feedname>
Adafruit_MQTT_Publish volume = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/volume");
 
// ======================================================================
// --- Variáveis Globais ---
long duracao = 0;
float distancia = 0;
int volumevar = 0;
// ======================================================================
// --- Void Setup ---
void setup() {
   
  Serial.begin(115200); // inicia comunicação serial com velocidade 115200
 
  Serial.println("Monitorar Volume Caixa D'água - Adafruit MQTT"); // escreve na serial
   
  // Conecta ao ponto de acesso WiFi
  Serial.println(); Serial.println();
  Serial.print("Conectando ao ");
  Serial.println(WLAN_SSID);
 
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
 
  Serial.println("WiFi Conectado");
  Serial.println("IP endereço: "); Serial.println(WiFi.localIP());
   
// ======================================================================
// --- Configuração IO ---
  pinMode(trigPin, OUTPUT); // pino D1 como saída para TRIGGER
  pinMode(echoPin, INPUT);  // pino D2 como entrada para ECHO
}
// ======================================================================
// --- void loop ---
void loop() {
  MQTT_connect();   // chama função para conexão com MQTT server
   
  digitalWrite(trigPin, LOW);    // Limpa trigPin
  delayMicroseconds(2);          // aguarda 2 microsegundos
 
  digitalWrite(trigPin, HIGH);   // Seta trigPin HIGH aguarda 
  delayMicroseconds(10);         // aguada 10 microsegundos
  digitalWrite(trigPin, LOW);    // Seta trigPin LOW 
   
  // Leitura do echoPin, retorna a onda de som em microsegundos
  duracao = pulseIn(echoPin, HIGH);
  distancia= duracao*0.034/2;
 
  if (distancia > 100){  // leitura mínima. Reservatório vazio
    distancia = 100;
  }
    if (distancia < 0){  // leitura máxima. Reservatório cheio
    distancia = 0;
  }
  volumevar = map(distancia, 1, 20, 100, 0); 
  /* Remapeia o range de leitura
   * Ao invés de ler de 20 a 0 cm, lerá de 100 a 0 para simular a porcentagem*/
 
  Serial.print("distancia: "); // imprime "distancia:"
  Serial.print(distancia);
  Serial.println(" cm");      // imprime a variavel distancia
  Serial.print("volume:");    // imprime "volume:"
  Serial.println(volumevar);  // imprime a variavel volume
   
  volume.publish(volumevar);     // publica variável "distância" em no feed "volume"
  // nossa saída será em níveis percentuais
   
  delay(3000); // aguarda 3 segundos
  /* Observação: A plataforma Adafruit IO só permite 30 publicações/minuto
   * na conta gratuita.*/
}
// ======================================================================
// --- Função para Conexão / Reconexão MQTT ---
  void MQTT_connect() {
    int8_t ret;
   
    // para de tentar reconexão quando consegue
    if (mqtt.connected()) {
      return;
    }
    Serial.print("Conectando ao MQTT... "); // imprime na serial
    // tenta conexão 5 vezes. Depois WatchDogTime!
    uint8_t retries = 5;
    while ((ret = mqtt.connect()) != 0) { // conectará quando retornar 0
      Serial.println(mqtt.connectErrorString(ret));
      Serial.println("Nova tentativa de conexão MQTT em 5 segundos...");
      mqtt.disconnect();
      delay(5000);  // aguarda 5 segundos
      retries--;
      if (retries == 0) { // trava e espera reset
        while (1);
      }
    }
    Serial.println("MQTT Conectado!"); // imprime na serial
  }
// ======================================================================
// --- FIM ---
