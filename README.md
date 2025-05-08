# MonitorWiFi ESP8266
Monitora a Internet WiFi que estiver configurado no dispositivo, possui um LED RGB para norificar o status da conexão com a Internet.

Compilado no ArduinoIDE 2.x

Hardware utilizado:
- ESP 8266 D1 Mini
- LED RGB
- Resistores

Configuração dos Pinos do ESP 8266:
- Pino D1: Led de conexão bem sucedida (VERDE)
- Pino D2: Led para alertar problema na conexão com a Internt (VERMELHO)
- Pino D4: Led Acende para exibir que está no modo AP, para configuração do WiFi a monitorar. (AZUL)
- Pino D3: Botão apaga o WiFi monitorado e permitir que seja configurado um novo. 

Funcionamento:
A primeira vez que for ligado ou após pressionado o botão Limpa WiFi, acessa o dispositivo pelo SSID MonWiFi_Config e senha 12345678.
Após conectado entre no endereço: http://192.168.4.1
Informe os dados do WiFi que será monitorado e salve a configuração.
Aguarde o reboot e o início do monitoramento.

Diagrama esquemático em: https://app.cirkitdesigner.com/project/ff460f93-f23a-4785-b744-997da97d6a5c

Diagrama esquemático:


**![image](https://github.com/user-attachments/assets/b5a0b02e-8980-4c0c-8fa3-6fdbd0ad8efe)
**
