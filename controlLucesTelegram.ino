#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoOTA.h>
#include <UniversalTelegramBot.h>
#include <Light.h>
#include <Timer.h>
#include "ntp.h"
#include "WIFIconfig.h"
#include "telegramConfig.h"

#define ROW_1 26
#define ROW_2 13
#define ROW_3 33
#define ROW_4 14
#define ROW_5 17
#define ROW_6 16

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
time_t getNtpTime();

// Lights
Light light_row_1(ROW_1);
Light light_row_2(ROW_2);
Light light_row_3(ROW_3);
Light light_row_4(ROW_4);
Light showCase(ROW_5);
Light light_row_6(ROW_6);

// Commands
const String GET_ALL_COMMANDS = "/comandos";
const String GET_ESTADO_LUCES = "/estado_luces";
const String SET_ALL_ON = "/encender_filas";
const String SET_ALL_OFF = "/apagar_filas";
const String SET_ON_ROW_MENU = "/menu_encender_fila";
const String SET_ON_ROW_1 = "/encender_fila_1";
const String SET_ON_ROW_2 = "/encender_fila_2";
const String SET_ON_ROW_3 = "/encender_fila_3";
const String SET_OFF_ROW_3 = "/apagar_fila_3";
const String SET_ON_ROW_4 = "/encender_fila_4";
const String SET_ON_ROW_5 = "/encender_escaparate";
const String SET_OFF_ROW_5 = "/apagar_escaparate";
const String SET_AUTOMATIC_ROW_5 = "/escaparate_automatico";
const String SHOWCASE_OPTIONS = "/opciones_escaparate";
const String SHOWCASE_OFFSET_ON = "/offset_encender_escaparate";
const String SHOWCASE_OFFSET_OFF = "/offset_apagar_escaparate";
const String SHOWCASE_OFFSET_ON_NUMBER = "/offset_showcase_on";
const String SHOWCASE_OFF_HOUR = "/offset_showcase_off";
const String FECHA_ACTUAL = "/fecha_actual";

// States
const String ON = "ON";
const String OFF = "OFF";

// Timers
TON *tCheckConnection;
TON *tCheckMessages;

// CONSTANTS:
const boolean START = true;
const boolean RESET = false;

void WIFIConnection()
{
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connecting to WiFi..");
    delay(10000);
    ESP.restart();
  }

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  Serial.println("Connected to the WiFi network");
}

void OTAConfig()
{
  ArduinoOTA.setHostname(CLIENT_NAME);
  ArduinoOTA.onStart([]()
                     {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type); });
  ArduinoOTA.onEnd([]()
                   { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    } });
  ArduinoOTA.begin();
}

void checkWifiConnection()
{
  if (tCheckConnection->IN(START))
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      WiFi.reconnect();
      delay(10000);

      if (WiFi.isConnected() == WL_CONNECTED)
      {
        WiFi.setAutoReconnect(true);
        WiFi.persistent(true);
        Serial.println("Connected to the WiFi network");
      }
    }

    tCheckConnection->IN(RESET);
  }
}

String getStatePrint(bool state)
{
  return state == 1 ? ON : OFF;
}

void writeResponse(String text, String chat_id)
{
  Serial.println(text);

  bot.sendMessage(chat_id, text, "Markdown");
}

void writeInlineMenu(String keyboardJson, String chat_id)
{
  bot.sendChatAction(chat_id, "typing");
  bot.sendMessageWithInlineKeyboard(chat_id, "Elige una opción de las siguientes", "", keyboardJson);
}

void handleLightCommands(String text, String chat_id)
{
  Serial.println(text);

  if (text == GET_ESTADO_LUCES)
  {
    String response = "Estado de las luces:\n\n";
    response += "Fila 1: " + String(getStatePrint(light_row_1.getState())) + "\n\n";
    response += "Fila 2: " + String(getStatePrint(light_row_2.getState())) + "\n\n";
    response += "Fila 3: " + String(getStatePrint(light_row_3.getState())) + "\n\n";
    response += "Fila 4: " + String(getStatePrint(light_row_4.getState())) + "\n\n";
    response += "Escaparate: " + String(getStatePrint(showCase.getState())) + "\n\n";

    writeResponse(response, chat_id);
  }

  if (text == SET_ALL_ON)
  {
    light_row_1.turnOn();
    light_row_2.turnOn();
    light_row_3.turnOn();
    light_row_4.turnOn();

    writeResponse("Todas las filas encendidas", chat_id);
  }

  if (text == SET_ALL_OFF)
  {
    light_row_1.turnOff();
    light_row_2.turnOff();
    // light_row_3.turnOff();
    light_row_4.turnOff();
    light_row_6.turnOff();

    writeResponse("Todas las filas apagadas", chat_id);
  }

  // Write the logic for the rest of the commands
  if (text == SET_ON_ROW_1)
  {
    light_row_1.turnOn();
    writeResponse("Fila 1 encendida", chat_id);
  }

  if (text == SET_ON_ROW_2)
  {
    light_row_2.turnOn();
    writeResponse("Fila 2 encendida", chat_id);
  }

  if (text == SET_ON_ROW_3)
  {
    light_row_3.turnOn();
    writeResponse("Fila 3 encendida", chat_id);
  }

  if (text == SET_OFF_ROW_3)
  {
    light_row_3.turnOff();
    writeResponse("Fila 3 apagada", chat_id);
  }

  if (text == SET_ON_ROW_4)
  {
    light_row_4.turnOn();
    writeResponse("Fila 4 encendida", chat_id);
  }

  if (text == SET_ON_ROW_5)
  {
    showCase.turnOffAutomatic();
    showCase.turnOn();
    writeResponse("Escaparate encendido", chat_id);
  }

  if (text == SET_OFF_ROW_5)
  {
    showCase.turnOffAutomatic();
    showCase.turnOff();
    writeResponse("Escaparate apagado", chat_id);
  }

  if (text == SET_AUTOMATIC_ROW_5)
  {
    showCase.turnOnAutomatic();
    writeResponse("Escaparate automatico", chat_id);
  }

  if (text == SHOWCASE_OPTIONS)
  {
    String keyboardJson =
        "[[{ \"text\" : \"Encender\", \"callback_data\" : \"/encender_escaparate\" }, { \"text\" : \"Apagar\", \"callback_data\" : \"/apagar_escaparate\" }],[{ \"text\" : \"Automatico\", \"callback_data\" : \"/escaparate_automatico\" }]]";
    writeInlineMenu(keyboardJson, chat_id);
  }

  if (text == SHOWCASE_OFFSET_ON)
  {
    String keyboardJson =
        "[[{ \"text\" : \"1\", \"callback_data\" : \"/offset_showcase_on_1\" }, { \"text\" : \"2\", \"callback_data\" : \"/offset_showcase_on_2\" }],[{ \"text\" : \"3\", \"callback_data\" : \"/offset_showcase_on_3\" }, { \"text\" : \"4\", \"callback_data\" : \"/offset_showcase_on_4\" }]]";
    writeInlineMenu(keyboardJson, chat_id);
  }

  if (text == SHOWCASE_OFFSET_OFF)
  {
    String keyboardJson =
        "[[{ \"text\" : \"1\", \"callback_data\" : \"/offset_showcase_off_1\" }, { \"text\" : \"2\", \"callback_data\" : \"/offset_showcase_off_2\" }],[{ \"text\" : \"3\", \"callback_data\" : \"/offset_showcase_off_3\" }, { \"text\" : \"4\", \"callback_data\" : \"/offset_showcase_off_4\" }]]";
    writeInlineMenu(keyboardJson, chat_id);
  }

  if (text.startsWith(SHOWCASE_OFFSET_ON_NUMBER))
  {
    setSunSetOffset(text.substring(text.length() - 1).toInt() * 60);
    writeResponse("Valor introducido correctamente", chat_id);
  }

  if (text.startsWith(SHOWCASE_OFF_HOUR))
  {
    setSunRiseHour(text.substring(text.length() - 1).toInt() * 60);
    writeResponse("Valor introducido correctamente", chat_id);
  }

  if (text == SET_ON_ROW_MENU)
  {
    String keyboardJson =
        "[[{ \"text\" : \"Fila 1\", \"callback_data\" : \"/encender_fila_1\" }],[{ \"text\" : \"Fila 2\", \"callback_data\" : \"/encender_fila_2\" }],[{ \"text\" : \"Fila 3\", \"callback_data\" : \"/encender_fila_3\" }],[{ \"text\" : \"Fila 4\", \"callback_data\" : \"/encender_fila_4\" }]]";
    writeInlineMenu(keyboardJson, chat_id);
  }

  if (text == FECHA_ACTUAL)
  {
    writeResponse(getDateAndHour(), chat_id);
    writeResponse("Es de noche: " + String(isNight()), chat_id);
    writeResponse("Time zone: " + String(getTimeZone()), chat_id);
    writeResponse("Escaparate: " + String(showCase.getState()), chat_id);
    writeResponse(getSunRiseAndSunSetCalculated(), chat_id);
  }
}

void handleNewMessages(int numNewMessages)
{
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++)
  {
    if (bot.messages[i].chat_id != JOSE_CHAT_ID && bot.messages[i].chat_id != MIGUEL_CHAT_ID && bot.messages[i].chat_id != CHEMA_CHAT_ID)
    {
      writeResponse("No tienes permisos para usar este bot", bot.messages[i].chat_id);
      continue;
    }

    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    if (text == GET_ALL_COMMANDS)
    {
      String welcome = "Control de luces oficina:\n\n";
      welcome += GET_ESTADO_LUCES + " - Lista el estado de las luces\n\n";
      welcome += SET_ALL_ON + " - Encender todas\n\n";
      welcome += SET_ALL_OFF + " - Apagar todas\n\n";
      welcome += SET_ON_ROW_MENU + " - Menu encender fila\n\n";
      welcome += SET_OFF_ROW_3 + " - Apagar fila 3\n\n";
      welcome += SHOWCASE_OPTIONS + " - Opciones escaparate\n\n";
      welcome += SHOWCASE_OFFSET_ON + " - Offset encendido\n\n";
      welcome += SHOWCASE_OFFSET_OFF + " - Hora apagado\n\n";
      welcome += FECHA_ACTUAL + " - Muestra la fecha actual\n\n";

      bot.sendMessage(chat_id, welcome);
    }

    handleLightCommands(text, chat_id);
  }
}

void setup()
{
  // Serial.begin(115200);

  Serial.println("Booting");

  WIFIConnection();

  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org

  OTAConfig();

  light_row_1.begin();
  light_row_2.begin();
  light_row_3.begin();
  light_row_4.begin();
  showCase.begin();
  light_row_6.begin();

  tCheckConnection = new TON(30000);
  tCheckMessages = new TON(1000);

  setupNtp();

  const String commands = F("["
                            "{\"command\":\"encender_filas\",  \"description\":\"Enciende todas las filas\"},"
                            "{\"command\":\"apagar_filas\", \"description\":\"Apaga todas las filas\"},"
                            "{\"command\":\"estado_luces\", \"description\":\"Obtiene el estado de todas las luces\"},"
                            "{\"command\":\"encender_fila_3\", \"description\":\"Enciende la fila 3\"},"
                            "{\"command\":\"apagar_fila_3\", \"description\":\"Apaga la fila 3\"},"
                            "{\"command\":\"opciones_escaparate\", \"description\":\"Opciones escaparate\"},"
                            "{\"command\":\"menu_encender_fila\",\"description\":\"Menu encender fila\"},"
                            "{\"command\":\"comandos\", \"description\":\"Obtiene todos los comandos\"}" // no comma on last command
                            "]");
  bot.setMyCommands(commands);

  showCase.turnOnAutomatic();
}

void loop()
{
  ArduinoOTA.handle();
  yield();
  checkWifiConnection();

  showCase.manageLightStateWithExternalCondition(isNight());

  if (tCheckMessages->IN(START))
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    tCheckMessages->IN(RESET);
  }
}