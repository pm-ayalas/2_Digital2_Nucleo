
#include <Bluepad32.h>
#include <Preferences.h>
#include "wire_protocol.h"
#define PLAYER_ID 0
static const char *nombre=PLAYER_ID==0?"FUEGO":"AGUA";
ControllerPtr pad=nullptr;
Preferences prefs;
uint8_t saved[6]={0},seq=0;
uint32_t lastSend=0,lastLog=0,packets=0;
bool savedValid=false;
void onConnected(ControllerPtr ctl){
 if(!ctl->isGamepad())return;
 auto prop=ctl->getProperties();
 if(pad || (savedValid && memcmp(saved,prop.btaddr,6))){
  Serial.println("Mando extra/no asignado ignorado. Apagalo. R borra el mando guardado.");return;
 }
 if(!savedValid){memcpy(saved,prop.btaddr,6);prefs.putBytes("mando",saved,6);savedValid=true;}
 pad=ctl;ctl->setColorLED(PLAYER_ID?0:255,PLAYER_ID?80:20,PLAYER_ID?255:0);
 Serial.printf("%s CONECTADO: %s\n",nombre,ctl->getModelName().c_str());
}
void onDisconnected(ControllerPtr ctl){
 if(pad==ctl){pad=nullptr;Serial.printf("%s DESCONECTADO\n",nombre);}
}
void setup(){
 Serial.begin(115200);Serial2.begin(115200,SERIAL_8N1,16,17);
 prefs.begin(PLAYER_ID?"agua1pad":"fuego1pad",false);
 savedValid=prefs.getBytes("mando",saved,6)==6;
 BP32.setup(&onConnected,&onDisconnected);BP32.enableVirtualDevice(false);
 Serial.printf("ESP32 %s. SHARE + PS. X=salto, O=pausa. R=reasignar.\n",nombre);
 Serial.println("USB=diagnostico; GPIO17=UART del juego; 115200 baudios.");
}
void loop(){
 BP32.update();
 if(Serial.available() && Serial.read()=='R'){
  Serial.println("Reiniciando :)");
  prefs.clear();BP32.forgetBluetoothKeys();delay(200);ESP.restart();
 }
 bool connected=pad && pad->isConnected();
 int axis=connected?pad->axisX():0;
 bool cross=connected && pad->a(),circle=connected && pad->b();
 uint8_t buttons=0;
 if(axis < -150)buttons|=1;else if(axis>150)buttons|=2;
 if(cross)buttons|=4; 
 if(circle)buttons|=8; 
 if(millis()-lastSend>=20){
  lastSend=millis();uint8_t p[8];
  WireMake(p,0x11+PLAYER_ID,buttons,connected?1:0,seq++);
  Serial2.write(p,8);packets++;
 }
 if(millis()-lastLog>=250){
  lastLog=millis();
  Serial.printf("%s %s | ejeX=%d X=%d O=%d | botones=0x%02X TX=%lu\n",
   nombre,connected?"CONECTADO":"DESCONECTADO",axis,(int)cross,(int)circle,
   (unsigned)buttons,(unsigned long)packets);
 }
 delay(1);
}
