# MonitorWiFi
Monitora o Internet WiFi que estiver configurado no dispositivo, possui um LED RGB para norificar o status da conexão com a Internet.

Hardware utilizado:
- ESP 8266 Mini
- LED RGB
- Resistores

Configuração dos Pinos do ESP 8266:
- Pino D1: Led de conexão bem sucedida (VERDE)
- Pino D2: Led para alertar problema na conexão com a Internt (VERMELHO)
- Pino D4: Led Acende para exibir que está no modo AP, para configuração do WiFi a monitorar.
- Pino D3: Botão Limpa o WiFi monitorado e permitir que seja configurado um novo.

Funcionamento:
A primeira vez que for ligado ou após pressionado o botão Limpa WiFi, acessa o dispositivo pelo SSID MonWiFi_Config e senha 12345678.
Após conectado entre no endereço: http://192.168.4.1
Informe os dados do WiFi que será monitorado e salve a configuração.
Aguarde o reboot e o início do monitoramento.
