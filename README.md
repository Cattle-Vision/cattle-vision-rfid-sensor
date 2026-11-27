# CattleVision RFID

Firmware desenvolvido para **ESP32**, responsável pela leitura de identificadores RFID utilizando o módulo **R200** e pela transmissão dos códigos das tags para um smartphone por meio de **Bluetooth Low Energy (BLE)**.

O projeto integra a identificação RFID com uma interface de comunicação sem fio, permitindo que os dados coletados sejam recebidos por um dispositivo móvel em tempo real.

## Funcionalidades

- **Leitura RFID:** comunicação com o leitor R200 por meio de UART.
- **Comunicação BLE:** disponibiliza um servidor Bluetooth Low Energy para conexão com smartphones.
- **Transmissão de identificadores:** envia o UID das tags RFID conectadas ao leitor através de notificações BLE.
- **Debounce de leitura:** evita transmissões excessivas da mesma tag, utilizando um intervalo de 3 segundos entre notificações consecutivas.
- **Feedback visual e sonoro:** utiliza um LED e um buzzer para sinalizar leituras bem-sucedidas.
- **Reconexão BLE:** reinicia automaticamente o advertising quando um dispositivo conectado é desconectado.

## Hardware utilizado

- ESP32 (ESP32-WROOM-32)
- Leitor RFID UHF R200
- Antena RFID compatível com o módulo R200
- LED de status
- Buzzer

## Conexões

| Componente | ESP32 |
|---|---|
| R200 TX | GPIO 16 (RX2) |
| R200 RX | GPIO 17 (TX2) |
| LED de status | GPIO 2 |
| Buzzer | GPIO 4 |

**Atenção:** verifique os níveis lógicos e os requisitos de alimentação do módulo R200 antes de realizar as conexões. Não conecte diretamente sinais de tensão incompatíveis com o ESP32.

## Comunicação RFID

O leitor R200 é inicializado utilizando a interface UART2 do ESP32, com os seguintes parâmetros:

- Baud rate: 115200
- RX: GPIO 16
- TX: GPIO 17

A leitura é realizada periodicamente, com intervalo de 200 ms entre as chamadas de polling.

Os identificadores RFID são tratados como sequências de 12 bytes e convertidos para uma representação hexadecimal de 24 caracteres.

## Comunicação Bluetooth Low Energy

O ESP32 opera como um servidor BLE com o nome:

`CattleVision-RFID`

O serviço GATT disponibiliza uma característica com suporte a leitura e notificações.

**UUID do serviço:**
```text
4fafc201-1fb5-459e-8fcc-c5c9c331914b
```

**UUID da característica:**
```text
beb5483e-36e1-4688-b7f5-ea07361b26a8
```

### Fluxo de comunicação

1. O smartphone procura dispositivos BLE próximos.
2. O dispositivo `CattleVision-RFID` é encontrado e conectado.
3. O smartphone habilita as notificações da característica RFID.
4. Quando uma tag é detectada, o ESP32 converte seu UID para hexadecimal.
5. O código é enviado ao smartphone por meio de uma notificação BLE.

O conteúdo transmitido é uma sequência ASCII contendo 24 caracteres hexadecimais, representando os 12 bytes do identificador RFID.

Exemplo:

```text
E28068940000501234567890
```

## Lógica de leitura e debounce

Para evitar múltiplas transmissões consecutivas enquanto uma mesma tag permanece no campo de leitura, o firmware implementa um mecanismo de debounce temporal.

- Uma nova tag é transmitida imediatamente quando seu UID é diferente do último UID enviado.
- Se a mesma tag continuar sendo detectada, uma nova transmissão só ocorre após 3 segundos.
- O intervalo é reiniciado a cada transmissão.

Dessa forma, o sistema permite reconhecer novamente uma tag que permanece próxima ao leitor, sem gerar notificações excessivas.

## Feedback visual e sonoro

Após uma leitura válida:

- O LED de status é desligado momentaneamente.
- O buzzer é acionado por 80 ms.
- O LED retorna ao estado correspondente à conexão BLE.

O LED também indica o estado da conexão:
- **Aceso:** smartphone conectado.
- **Apagado:** nenhum smartphone conectado.

## Instalação e execução

### 1. Preparação

Instale o [Arduino IDE](https://www.arduino.cc/en/software) e configure o suporte à placa ESP32.

### 2. Dependências

O firmware utiliza as seguintes bibliotecas:

- `BLEDevice`
- `BLEServer`
- `BLEUtils`
- `BLE2902`
- `R200.h`

As bibliotecas BLE são disponibilizadas pelo framework ESP32 Arduino. A biblioteca `R200` deve estar disponível no ambiente de desenvolvimento.

### 3. Configuração

Configure a placa ESP32 correspondente ao hardware utilizado e conecte o dispositivo ao computador.

### 4. Upload

Abra o arquivo principal do projeto no Arduino IDE, selecione a porta serial correspondente ao ESP32 e faça o upload do firmware.

### 5. Utilização

Após a inicialização:

1. O leitor RFID R200 será inicializado.
2. O servidor BLE será iniciado.
3. O ESP32 começará a anunciar o dispositivo `CattleVision-RFID`.
4. Conecte um smartphone utilizando um aplicativo de exploração BLE, como o [nRF Connect](https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-mobile).
5. Habilite as notificações da característica RFID.
6. Aproxime uma tag RFID da antena e acompanhe o recebimento do identificador.

## Monitoramento

O firmware disponibiliza mensagens de depuração pela porta serial, configurada em 115200 baud.

As mensagens permitem acompanhar:
- Inicialização do leitor RFID.
- Inicialização do servidor BLE.
- Conexões e desconexões BLE.
- Identificadores RFID detectados.
- Notificações transmitidas ao smartphone.

---

**CattleVision RFID**  
Firmware de identificação RFID e comunicação BLE baseado em ESP32.
