# tcc-animaltracker-esp32
Trabalho de Conclusão de Curso desenvolvido para coleta de dados de beacons Bluetooth e transmissão via LoRa.

## Pré requisitos

* Arduino IDE
* 2 Placas de desenvolvimento Heltec WiFi LoRa 32 (V2) com microcontrolador ESP32
* RaspberryPi para configuração do servidor local


## Instalação

### Estação Coletora e Estação Central

O primeiro passo é configurar a IDE do Arduino para poder configurar a placa de desenvolvimento. 

Nesse projeto, foi utilizado esse [tutorial](https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/blob/master/InstallGuide/windows.md) para configurar a placa.

Com a IDE configurada é possível programar uma placa de desenvolvimento para ser a Estação Coletora e outra para ser a Estação Central.

### Servidor local

Para o servidor local funcionar corretamente é necessário instalar 3 principais componentes na RaspberryPi: Broker MQTT Mosquitto, Node-RED e Servidor Apache.

* Mosquitto: Para instalar o Broker Mosquitto entre com os seguintes comandos: 

```
sudo apt update
sudo apt install -y mosquitto mosquitto-clients
```

Você terá que apertar **Y** e depois **Enter** para confirmar a instalação.

Para fazer o Mosquitto inicializar automaticamente quando a RaspberryPi ligar digite:

```
sudo systemctl enable mosquitto.service
```

Pronto, agora é só digitar o comando abaixo que o broker irá inicializar.

```
mosquitto
```

* Node-RED: Para instalar o Node-RED na RaspberryPi entre com o seguinte comando:

```
bash <(curl -sL https://raw.githubusercontent.com/node-red/raspbian-deb-package/master/resources/update-nodejs-and-nodered)
```

Depois de instalada é só inicializar à partir do comando


```
node-red
```

Depois disso, é só importar o arquivo [FlowNodeRed.json](https://github.com/LucasSossai/tcc-animaltracker-esp32/blob/master/FlowNodeRed.json) para seu flow no Node-RED

* Servidor Apache: Foi utilizado esse [tutorial](https://www.youtube.com/watch?v=kQ0HoLva9Yc&t=3s) para instalação dos componentes necessários. Depois de instalado, é utilizado o IP local no Node-RED para inserir os dados no servidor MySQL.


## Autor

* **Lucas Sossai** - [LucasSossai](https://github.com/LucasSossai)



