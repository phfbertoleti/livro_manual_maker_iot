import Adafruit_DHT
import RPi.GPIO as GPIO
import gspread
from oauth2client.service_account import ServiceAccountCredentials
from datetime import datetime
import time


# Define o tipo de sensor de temperatura e umidade utilizado
sensor = Adafruit_DHT.DHT22

#configura GPIO
GPIO.setmode(GPIO.BOARD)

# Define o GPIO da Raspberry Pi que esta conectado ao
# pino de dados do sensor
pino_sensor = 25

#carrega as credenciais para uso das APIs do Google
scope = ["https://spreadsheets.google.com/feeds","https://www.googleapis.com/auth/spreadsheets","https://www.googleapis.com/auth/drive.file","https://www.googleapis.com/auth/drive"] 
creds = ServiceAccountCredentials.from_json_keyfile_name('credenciais.json', scope) 
client = gspread.authorize(creds)

#Informa qual planilha do Google Sheets deve ser acessada
#Sera considerado que todas as informacoes serao escritas somente na primeira aba da planilha
sheet = client.open('Planilha teste').sheet1


#Por tempo indeterminado, faz o seguinte:
#- leitura do sensor
#- envio das informacoes do sensor (juntamente com a data e hora da medicao) para a planilha do Google Sheets
#- aguarda um minuto para a proxima medicao

while True:
    #obtem a data e hora atuais no formato
    #dd/MM/aaaa  hh:mm:ss
    now = datetime.now()
    datahora = now.strftime("%d/%m/%Y %H:%M:%S")

    #faz leitura do sensor DHT22
    umid, temp = Adafruit_DHT.read_retry(sensor, pino_sensor)

    #Se a leitura for valida, envia data, hora e leituras do
    # sensor para a planilha no Google Sheets
    if umid is not None and temp is not None:
        linha_a_ser_adicionada = [datahora, str(temp), str(umid)]
        sheet.append_row(linha_a_ser_adicionada)

    #aguarda um minuto ate proximo envio
    time.sleep(60)

