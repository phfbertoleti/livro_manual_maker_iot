
#include <WiFi.h> 

/* Header para uso da biblioteca PubSubClient */
#include <PubSubClient.h>  

/* Header para uso da biblioteca do sensor de distância ultrassônico */
#include <Ultrasonic.h>

/* Definições - serial de debug */
#define SERIAL_DEBUG_BAUDRATE          115200
#define NUM_PULA_LINHAS_SERIAL_DEGUB   80

/* Definições - sensor ultrassônico */
#define GPIO_TRIGGER    23
#define GPIO_ECHO       22

/* Definições - medição de vazão */
#define PI_APROXIMADO            3.1415927
#define NUM_MEDIDAS_DISTANCIA    100

/* Definições - área do reservatório: */
/* descomentar somente o tipo de área do seu reservatório 
   (circulo ou retângulo */
//#define RETANGULO
#define CIRCULO
 
/* Medidas se for um retangulo: */
#define RETANGULO_BASE       1  //[m]
#define RETANGULO_ALTURA     1  //[m]
 
/* Medidas se for um circulo: */
#define CIRCULO_RAIO         0.05  //[m]

/* Definições - MQTT */
/* IMPORTANTE: recomendamos fortemente alterar os nomes
               desses tópicos. Caso contrário, há grandes
               chances de você enviar e receber mensagens de um ESP32
               de outra pessoa.
*/
/* Tópico MQTT para envio de informações do ESP32 para broker MQTT */
#define TOPICO_PUBLISH   "INCB_ESP32_envia_vazao_volume"  

/* id mqtt (para identificação de sessão) */
/* IMPORTANTE: este deve ser único no broker (ou seja, 
               se um client MQTT tentar entrar com o mesmo 
               id de outro já conectado ao broker, o broker 
               irá fechar a conexão de um deles).
*/
#define ID_MQTT  "INCB_Cliente_MQTT_vazao_volume"     

/* Variáveis e objetos globais */
Ultrasonic ultrasonic(GPIO_TRIGGER, GPIO_ECHO);
float area_perfil_reservatorio;  
float volume_total_retirado = 0.0;
float vazao_calculada = 0.0;

/*  Variáveis e constantes globais */
/* SSID / nome da rede WI-FI que deseja se conectar */
const char* SSID = " "; 

/*  Senha da rede WI-FI que deseja se conectar */
const char* PASSWORD = " "; 
  
/* URL do broker MQTT que deseja utilizar */
const char* BROKER_MQTT = "broker.hivemq.com"; 

/* Porta do Broker MQTT */
int BROKER_PORT = 1883;

/* wi-fi */
WiFiClient espClient;

/* MQTT */
PubSubClient MQTT(espClient);
 
/* Prototypes */
float calcula_area_perfil_reservatorio(void);
float mede_distancia_em_metros(void);
float media_distancias(void);
float calcula_vazao_e_volume_retirado(void);
void init_wifi(void);
void init_mqtt(void);
void reconnect_wifi(void); 
void verifica_conexoes_wifi_mqtt(void);

/*
 * Implementações
 */
 
/* Função: cálculo da área do perfil do reservatório
 * Parâmetros: nenhum
 * Retorno: área calculada
 */
 float calcula_area_perfil_reservatorio(void)
 {
    float area_calc = 0.0;
    
    #ifdef RETANGULO
      area_calc = RETANGULO_BASE * RETANGULO_ALTURA;    
    #endif
 
    #ifdef CIRCULO
      area_calc = PI_APROXIMADO * CIRCULO_RAIO * CIRCULO_RAIO;    
    #endif
 
    return area_calc; 
 }
 
 /* Função: Média de medidas de distância
  * Parâmetros: nenhum
  * Retorno: média das medidas de distância
  */
float media_distancias(void)
{
    int i;
    float soma_medidas = 0.0;
    float media = 0.0;

    for(i=0; i<NUM_MEDIDAS_DISTANCIA; i++)
        soma_medidas = soma_medidas + mede_distancia_em_metros();     
 
    media = (soma_medidas / NUM_MEDIDAS_DISTANCIA);    
    return media;    
}
 
 
 /* Função: mede distancia em metros
  * Parametros: nenhum
  * Retorno: distancia (m)
 */
float mede_distancia_em_metros(void) 
{
    float cm_msec = 0.0;
    float dist_metros = 0.0;
    long microsec = ultrasonic.timing();
 
    cm_msec = ultrasonic.convert(microsec, Ultrasonic::CM);
    dist_metros = (cm_msec / 100.0);
    
    return dist_metros; 
}
 
 
/*  Função: calcula vazão e volume total retirado do reservatório
 *  Parâmetros: nenhum
 *  Retorno: nenhum
 */
float calcula_vazao_e_volume_retirado(void)
{
    float distancia_t2;
    float distancia_t1;    
    float variacao_distancia;
    float volume_em_litros;
    char i;
        
    /* faz as medições de distancias */
    distancia_t1 = media_distancias(); 
    delay(1000);   
    distancia_t2 = media_distancias();

    /* Calcula vazão e volume total retirado */
    variacao_distancia = ((distancia_t2 - distancia_t1) / 100);   //[m]
    vazao_calculada = (area_perfil_reservatorio * variacao_distancia); //[m³/s]
    volume_total_retirado = volume_total_retirado + vazao_calculada; //[m³]
    volume_em_litros = (volume_total_retirado*1000.0); //[l]

    /* Escreve medições no Serial monitor (serial de debug) */
    for (i=0; i<NUM_PULA_LINHAS_SERIAL_DEGUB; i++)
        Serial.println(""); 
      
    Serial.println("[DADOS DA MEDIÇÃO]");
    Serial.print("Vazao calculada: ");
    Serial.print(vazao_calculada);
    Serial.print(" m^3/s");
    Serial.println("");
    Serial.print("Variacao de distancia: ");
    Serial.print(variacao_distancia*100);
    Serial.print(" cm");    
    Serial.println("");
    Serial.print("Volume total retirado: ");
    Serial.print(volume_em_litros);
    Serial.print(" l");
    Serial.println("");
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


void setup()
{
    Serial.begin(SERIAL_DEBUG_BAUDRATE);
 
    /* calcula área do perfil (com base em parametros fornecidos) 
       nas definições */
    area_perfil_reservatorio = calcula_area_perfil_reservatorio();

    /* Inicializa variável de volume total retirado */
    volume_total_retirado = 0.0;

    /* Conecta-se no wi-fi e MQTT */
    init_wifi();
    init_mqtt();
}
 
void loop()
{
    char msg_mqtt[100]={0};
    
    /* Calcula vazão e volume total retirado do reservatório */ 
    calcula_vazao_e_volume_retirado();

    /* garante que haja conectividade wi-fi e MQTT */  
    verifica_conexoes_wifi_mqtt();

    /* envia vazão e volume total retirado do reservatório 
       via MQTT pra nuvem */
    sprintf(msg_mqtt, "Vazao: %.2fm^3/s - Volume: %.2fm^3", vazao_calculada, volume_total_retirado);
    MQTT.publish(TOPICO_PUBLISH, msg_mqtt);

    /* Faz keep alive do MQTT */
    MQTT.loop();
}
