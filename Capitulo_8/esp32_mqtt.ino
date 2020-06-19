//Programa: comunicação MQTT com ESP32
//Autor: Pedro Bertoleti
 
/* Headers */ 
#include <WiFi.h> /* Header para uso das funcionalidades de wi-fi do ESP32 */
#include <PubSubClient.h>  /*  Header para uso da biblioteca PubSubClient */
 
/* Defines do MQTT */
/* IMPORTANTE: recomendamos fortemente alterar os nomes
               desses tópicos. Caso contrário, há grandes
               chances de você enviar e receber mensagens de um ESP32
               de outra pessoa.
*/
/* Tópico MQTT para recepção de informações do broker MQTT para ESP32 */
#define TOPICO_SUBSCRIBE "INCB_ESP32_recebe_informacao"   

/* Tópico MQTT para envio de informações do ESP32 para broker MQTT */
#define TOPICO_PUBLISH   "INCB_ESP32_envia_informacao"  

/* id mqtt (para identificação de sessão) */
/* IMPORTANTE: este deve ser único no broker (ou seja, 
               se um client MQTT tentar entrar com o mesmo 
               id de outro já conectado ao broker, o broker 
               irá fechar a conexão de um deles).
*/
#define ID_MQTT  "INCB_Cliente_MQTT"     

/*  Variáveis e constantes globais */
/* SSID / nome da rede WI-FI que deseja se conectar */
const char* SSID = "SSID"; 
/*  Senha da rede WI-FI que deseja se conectar */
const char* PASSWORD = "SENHA"; 
  
/* URL do broker MQTT que deseja utilizar */
const char* BROKER_MQTT = "broker.hivemq.com"; 
/* Porta do Broker MQTT */
int BROKER_PORT = 1883;
 
 
/* Variáveis e objetos globais */
WiFiClient espClient;
PubSubClient MQTT(espClient);
  
//Prototypes
void init_serial(void);
void init_wifi(void);
void init_mqtt(void);
void reconnect_wifi(void); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void verifica_conexoes_wifi_mqtt(void);
 
/* 
 *  Implementações das funções
 */
void setup() 
{
    init_serial();
    init_wifi();
    init_mqtt();
}
  
/* Função: inicializa comunicação serial com baudrate 115200 (para fins de monitorar no terminal serial 
*          o que está acontecendo.
* Parâmetros: nenhum
* Retorno: nenhum
*/
void init_serial() 
{
    Serial.begin(115200);
}
 
/* Função: inicializa e conecta-se na rede WI-FI desejada
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void init_wifi(void) 
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
    reconnect_wifi();
}
  
/* Função: inicializa parâmetros de conexão MQTT(endereço do  
 *         broker, porta e seta função de callback)
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void init_mqtt(void) 
{
    /* informa a qual broker e porta deve ser conectado */
    MQTT.setServer(BROKER_MQTT, BROKER_PORT); 

    /* atribui função de callback (função chamada quando qualquer informação do 
    tópico subescrito chega) */
    MQTT.setCallback(mqtt_callback);            
}
  
/* Função: função de callback 
 *          esta função é chamada toda vez que uma informação de 
 *          um dos tópicos subescritos chega)
 * Parâmetros: nenhum
 * Retorno: nenhum
 * */
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    String msg;
 
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }

    Serial.print("[MQTT] Mensagem recebida: ");
    Serial.println(msg);     
}
  
/* Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
 *          em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void reconnect_mqtt(void) 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}
  
/* Função: reconecta-se ao WiFi
 * Parâmetros: nenhum
 * Retorno: nenhum
*/
void reconnect_wifi() 
{
    /* se já está conectado a rede WI-FI, nada é feito. 
       Caso contrário, são efetuadas tentativas de conexão */
    if (WiFi.status() == WL_CONNECTED)
        return;
         
    WiFi.begin(SSID, PASSWORD);
     
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
   
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}
 
/* Função: verifica o estado das conexões WiFI e ao broker MQTT. 
 *         Em caso de desconexão (qualquer uma das duas), a conexão
 *         é refeita.
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void verifica_conexoes_wifi_mqtt(void)
{
    /* se não há conexão com o WiFI, a conexão é refeita */
    reconnect_wifi(); 

    /* se não há conexão com o Broker, a conexão é refeita */
    if (!MQTT.connected()) 
        reconnect_mqtt(); 
} 
 
/* programa principal */
void loop() 
{   
    /* garante funcionamento das conexões WiFi e ao broker MQTT */
    verifica_conexoes_wifi_mqtt();

    /* Envia frase ao broker MQTT */
    MQTT.publish(TOPICO_PUBLISH, "ESP32 se comunicando com MQTT");
 
    /* keep-alive da comunicação com broker MQTT */    
    MQTT.loop();

    /* Aguarda 1 segundo para próximo envio */
    delay(1000);   
}

