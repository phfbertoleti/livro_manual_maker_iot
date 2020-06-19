import paho.mqtt.client as mqtt
import sys

#definicoes do MQTT
Broker = "broker.hivemq.com" #broker publico utilizado.
porta_broker = 1883 #porta utilizada para comunicacao com o broker MQTT
                    #utilize a porta 1883 para comunicacao com conexao nao segura
keep_alive_broker = 60 #tempo (em segundos) do keep-alive
topico_subscribe = "MQTTRaspPiINCB"  #topico MQTT que o programa ira "ouvir" (fazer subscribe)
                                     #dica: troque o nome do topico por algo "unico", 
                                     #Dessa maneira, ninguem ira saber seu topico de
                                     #subscribe e interferir em seus testes

#Callback - conexao ao broker realizada
def on_connect(client, userdata, flags, rc):
    print("[STATUS] Conectado ao Broker.")
    #faz subscribe automatico no topico
    client.subscribe(topico_subscribe)

#Callback - mensagem recebida do broker
#toda vez que uma mensagem for recebida do broker, esta funcao sera chamada
def on_message(client, userdata, msg):
	MensagemRecebida = str(msg.payload)
	print("[MSG RECEBIDA] Topico: "+msg.topic+" / Mensagem: "+MensagemRecebida)

#programa principal:
try:
        print("[STATUS] Inicializando MQTT...")
        #inicializa MQTT:

        #cria client MQTT e define funcoes de callback de conexao (client.on_connect) 
        #e recepcao de dados recebidos (client.on_message)
        client = mqtt.Client()
        client.on_connect = on_connect
        client.on_message = on_message
        #faz a conexao ao broker MQTT
        client.connect(Broker, porta_broker, keep_alive_broker)
        #mantem o MQTT funcionando por tempo indeterminado, sendo que todas as
        #mensagens recebidas vao fazer a funcao de callback de dados recebidos 
        #(on_message) ser chamada
        client.loop_forever()
except KeyboardInterrupt:
        print "\nCtrl+C pressionado, encerrando aplicacao e saindo..."
        sys.exit(0)
