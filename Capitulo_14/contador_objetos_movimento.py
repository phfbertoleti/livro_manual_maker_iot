
import datetime
import math
import cv2
import numpy as np
import http.client, urllib.parse
import time

#variaveis globais
write_api_key_thingspeak = ''  #coloque aqui a Write API Key de seu canal ThingSpeak
width = 0
height = 0
contador_entradas = 0
contador_saidas = 0
area_contorno_limite_minimo = 3000  #este valor eh empirico. Ajuste-o conforme sua necessidade 
threshold_binarizacao = 70  #este valor eh empirico, Ajuste-o conforme sua necessidade
offset_linhas_referencia = 150  #este valor eh empirico. Ajuste-o conforme sua necessidade.
total_objetos_contados = 0   #variavel que contem o total de objetos em movimento contados (entrando ou saindo da zona monitorada)

#Verifica se o corpo detectado esta entrando da sona monitorada
def testa_interseccao_entrada(y, coordenada_y_linha_entrada, coordenada_y_linha_saida):
    diferenca_absoluta = abs(y - coordenada_y_linha_entrada)	

    if ((diferenca_absoluta <= 2) and (y < coordenada_y_linha_saida)):
        return 1
    else:
        return 0

#Verifica se o corpo detectado esta saindo da sona monitorada
def testa_interseccao_saida(y, coordenada_y_linha_entrada, coordenada_y_linha_saida):
    diferenca_absoluta = abs(y - coordenada_y_linha_saida)	

    if ((diferenca_absoluta <= 2) and (y > coordenada_y_linha_entrada)):
        return 1
    else:
        return 0

camera = cv2.VideoCapture(0)

#forca a camera a ter resolucao 640x480
camera.set(3,640)
camera.set(4,480)

primeiro_frame = None

#faz algumas leituras de frames antes de consierar a analise
#motivo: algumas camera podem demorar mais para se "acosumar a luminosidade" quando ligam, capturando frames consecutivos com muita variacao de luminosidade. Para nao levar este efeito ao processamento de imagem, capturas sucessivas sao feitas fora do processamento da imagem, dando tempo para a camera "se acostumar" a luminosidade do ambiente

for i in range(0,20):
    (grabbed, Frame) = camera.read()

timestamp_envio_thingspeak = int(time.time())

while True:
    #le primeiro frame e determina resolucao da imagem
    (grabbed, Frame) = camera.read()
    height = np.size(Frame,0)
    width = np.size(Frame,1)

    #se nao foi possivel obter frame, nada mais deve ser feito
    if not grabbed:
        break

    #converte frame para escala de cinza e aplica efeito blur (para realcar os contornos)
    frame_gray = cv2.cvtColor(Frame, cv2.COLOR_BGR2GRAY)
    frame_gray = cv2.GaussianBlur(frame_gray, (21, 21), 0)

    #como a comparacao eh feita entre duas imagens subsequentes, se o primeiro frame eh nulo (ou seja, primeira "passada" no loop), este eh inicializado
    if primeiro_frame is None:
        primeiro_frame = frame_gray
        continue

    #ontem diferenca absoluta entre frame inicial e frame atual (subtracao de background)
    #alem disso, faz a binarizacao do frame com background subtraido 
    frame_delta = cv2.absdiff(primeiro_frame, frame_gray)
    frame_threshold = cv2.threshold(frame_delta, threshold_binarizacao, 255, cv2.THRESH_BINARY)[1]
    
    #faz a dilatacao do frame binarizado, com finalidade de elimunar "buracos" / zonas brancas dentro de contornos detectados. 
    #Dessa forma, objetos detectados serao considerados uma "massa" de cor preta 
    #Alem disso, encontra os contornos apos dilatacao.
    frame_threshold = cv2.dilate(frame_threshold, None, iterations=2)
    
    #Abaixo estao as duas chamadas de cv2.findContours possiveis. 
    #Utilize aquela que funcionar com sua versão de OpenCV
    #_, cnts, _ = cv2.findContours(frame_threshold.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    cnts, _ = cv2.findContours(frame_threshold.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    qtde_contornos = 0

    #desenha linhas de referencia 
    coordenada_y_linha_entrada = int((height / 2)-offset_linhas_referencia)
    coordenada_y_linha_saida = int((height / 2)+offset_linhas_referencia)
    cv2.line(Frame, (0,coordenada_y_linha_entrada), (width,coordenada_y_linha_entrada), (255, 0, 0), 2)
    cv2.line(Frame, (0,coordenada_y_linha_saida), (width,coordenada_y_linha_saida), (0, 0, 255), 2)


    #Varre todos os contornos encontrados
    for c in cnts:
        #contornos de area muto pequena sao ignorados.
        if cv2.contourArea(c) < area_contorno_limite_minimo:
            continue

        #Para fins de depuracao, contabiliza numero de contornos encontrados
        qtde_contornos = qtde_contornos+1    

        #obtem coordenadas do contorno (na verdade, de um retangulo que consegue abrangir todo ocontorno) e
        #realca o contorno com um retangulo.
        (x, y, w, h) = cv2.boundingRect(c) #x e y: coordenadas do vertice superior esquerdo
                                           #w e h: respectivamente largura e altura do retangulo

        cv2.rectangle(Frame, (x, y), (x + w, y + h), (0, 255, 0), 2)

        #determina o ponto central do contorno e desenha um circulo para indicar
        coordenada_x_centroide_contorno = int((x+x+w)/2)
        coordenada_y_centroide_contorno = int((y+y+h)/2)
        ponto_central_contorno = (coordenada_x_centroide_contorno,coordenada_y_centroide_contorno)
        cv2.circle(Frame, ponto_central_contorno, 1, (0, 0, 0), 5)
        
        #testa interseccao dos centros dos contornos com as linhas de referencia
        #dessa forma, contabiliza-se quais contornos cruzaram quais linhas (num determinado sentido)
        if (testa_interseccao_entrada(coordenada_y_centroide_contorno,coordenada_y_linha_entrada,coordenada_y_linha_saida)):
            contador_entradas += 1

        if (testa_interseccao_saida(coordenada_y_centroide_contorno,coordenada_y_linha_entrada,coordenada_y_linha_saida)):  
            contador_saidas += 1

        #Se necessario, descomentar as lihas abaixo para mostrar os frames utilizados no processamento da imagem
        #cv2.imshow("Frame binarizado", frame_threshold)
        #cv2.waitKey(1);
        #cv2.imshow("Frame com subtracao de background", frame_delta)
        #cv2.waitKey(1);


    print("Contornos encontrados: "+str(qtde_contornos))

    #contabiliza todos os objetos em movimento que entraram e sairam da zona monitorada
    total_objetos_contados = contador_entradas + contador_saidas

    #Escreve na imagem o numero de pessoas que entraram ou sairam da area vigiada
    cv2.putText(Frame, "Entradas: {}".format(str(contador_entradas)), (10, 50),
                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (250, 0, 1), 2)
    cv2.putText(Frame, "Saidas: {}".format(str(contador_saidas)), (10, 70),
                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)
    cv2.imshow("Original", Frame)
    cv2.waitKey(1);

    #Verifica se deve enviar a contagem para o ThingSpeak
    if (int(time.time()) - timestamp_envio_thingspeak >= 15):
        params = urllib.parse.urlencode({'field1': total_objetos_contados, 'key':write_api_key_thingspeak})
        headers = {"Content-typZZe": "application/x-www-form-urlencoded","Accept": "text/plain"}
        conn = http.client.HTTPConnection("api.thingspeak.com:80")
        try:
            conn.request("POST", "/update", params, headers)
            response = conn.getresponse()
            print (response.status, response.reason)
            data = response.read()
            conn.close()
        except:
            print ("Erro ao enviar ao ThingSpeak")
        
        timestamp_envio_thingspeak = int(time.time())


# cleanup the camera and close any open windows
camera.release()
cv2.destroyAllWindows()
