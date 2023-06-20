# Light control
## Introduction
This project is a simple light control using a telegram bot for controll it remotely.

## Requirements
* ESP32
* Telegram bot
* 6 Relay

## PINOUT
| Name                | Name in board | GPIO/IDE Arduino name |
|---------------------|---------------|-----------------------|
| Row 1               | IO26          | 26                    |
| Row 2               | TCK           | 13                    |
| Row 3               | IO33          | 33                    |
| Row 4               | TMS           | 14                    |
| Row 5               | IO17          | 17                    |
| Row 6               | IO16          | 16                    |

## Bot commands
Go to @BotFather and put /setcommands in chat. Then select your bot an paste this list:

encender_filas - Enciende todas las filas
apagar_filas - Apagar todas las filas
encender_fila_3 - Enciende fila 3
estado_luces - Obtiene el estado de todas las luces
encender_escaparate - Enciende escaparate
apagar_escaparate - Apaga escaparate
comandos - Obtiene todos los comandos
