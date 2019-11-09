// contrôle GROT avec le joystick gris sur Aruino Leonardo et SSC32
// droite et gauche sont définis en regardant de face
// 2017

#include <AltSoftSerial.h>
// Arduino Leonardo   5 TX      13 RX
AltSoftSerial altSerial;

#include <XBee.h>
#define NB_CHANNEL 12

boolean debug = true;

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
// create reusable response objects for responses we expect to handle
Rx16Response rx16 = Rx16Response();

uint8_t option = 0;
int TxVal[NB_CHANNEL];

unsigned long timer = 0;

int t = 10;
boolean transitionReveil = false;

int rotation;    //valeurs en microsec pour les servos
int s_lsup_g;
int s_lsup_d;
int s_linf;
int s_sou_g;
int s_sou_d;
int s_mach;
int s_arcin;
int s_arcex;      //INITIALISER VALEURS
int s_oeil_g;
int s_oeil_d;
int s_cou_d;
int s_cou_g;

int delta;

void setup()
{
  Serial1.begin(115200);
  Serial.begin(9600);
  altSerial.begin(19200);

  xbee.setSerial(altSerial);

  delay(2000);

  // Serial.println("go");
}

void loop()
{

  xbee.readPacket();

  if (xbee.getResponse().isAvailable());
  if (xbee.getResponse().getApiId() == RX_16_RESPONSE) {
    xbee.getResponse().getRx16Response(rx16);
    //      option = rx16.getOption();

    readData();

    if (debug)
    {
      Serial.print("TxVal = ");
      for (int i = 0; i < NB_CHANNEL; i++) {
        Serial.print(TxVal[i]);
        Serial.print(" ");
      }
      Serial.println();

      if (timer != 0) {
        Serial.print("timer : ");
        Serial.println(millis() - timer);
      }
      timer = millis();
    }

    potards();
  }
}

void potards()
{
  delta = map(TxVal[2], 0, 1023, -500, 500);

  rotation = map (TxVal[0], 0, 1023, 1000, 1800);

  s_cou_d = map(TxVal[1] - delta, 0, 1023, 1750, 1200);
  s_cou_d = constrain (s_cou_d, 1200, 1750);
  s_cou_g = map(TxVal[1] + delta, 0, 1023, 1250, 1800);
  s_cou_g = constrain (s_cou_g, 1250, 1800);

  s_mach = map(TxVal[3], 0, 1023, 1700, 1150);

    s_sou_g = map(TxVal[4] , 0, 1023, 1150, 2100);
    s_sou_g = constrain(s_sou_g, 1200, 2100);
    s_sou_d = map(TxVal[4] , 0, 1023, 1800, 900);
    s_sou_d = constrain(s_sou_d, 900, 1800);

  s_linf = map(TxVal[4], 0, 1023, 1100, 1850);
  s_linf = constrain(s_linf, 1150, 1850);

    s_lsup_g = map(TxVal[5] , 0, 1023, 1100, 1900);
    s_lsup_g = constrain(s_lsup_g, 1100, 1900);
    s_lsup_d = map(TxVal[5] , 0, 1023, 1800, 1000);
    s_lsup_d = constrain(s_lsup_d, 1000, 1800);


  if (TxVal[9] == 1)
  {
    if (TxVal[11] == 1) s_oeil_g = 1950;
    else {
      s_oeil_d = 1900;
      s_oeil_g = 1950;
    }
  }
  else
  {
    s_oeil_g = map(TxVal[7], 0, 1023, 1450, 1950);
    s_oeil_g = constrain(s_oeil_g, 1450, 1950);
    s_oeil_d = map(TxVal[7], 0, 1023, 1550, 1900);
    s_oeil_d = constrain(s_oeil_d, 1550, 1900);
  }

  s_arcin = map(TxVal[5], 0, 1023, 1400, 1800);

  s_arcex = map(TxVal[6], 0, 1023, 1750, 1950);

  actionAll();
}

void actionAll()
{
  action (16, rotation, t);
  action (17, s_cou_d, t);
  action (18, s_cou_g, t);
  action (9, s_mach, t);
  action (1, s_sou_g, t);
  action (8, s_sou_d, t);
  action (0, s_linf, t);
  action (7, s_lsup_g, t);
  action (4, s_lsup_d, t);
  action (2, s_oeil_g, t);
  action (5, s_oeil_d, t);
  action (3, s_arcin, t);
  action (6, s_arcex, t);
}

void action(int servo, int pos, int time) {
  Serial1.print("#");
  Serial1.print(servo);
  Serial1.print(" P");
  Serial1.print(pos);
  Serial1.print(" T");
  Serial1.println(time);
}

void readData() {
  if (rx16.getDataLength() == NB_CHANNEL * 2) {
    int tabIndex = 0;
    for (int i = 0; i < rx16.getDataLength(); i = i + 2) {
      TxVal[tabIndex] = rx16.getData(i + 1) + (rx16.getData(i) * 256);
      tabIndex++;
    }
  }
}
