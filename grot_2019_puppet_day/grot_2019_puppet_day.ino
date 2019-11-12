// contrôle GROT avec le joystick gris sur Aruino Leonardo et SSC32
// droite et gauche sont définis en regardant de face
// 2017

#include <AltSoftSerial.h>
#include <SPI.h>
#include <SD.h>

// Arduino Leonardo   5 TX      13 RX
AltSoftSerial altSerial;

#include <XBee.h>
#define NB_CHANNEL 12

#define MINUTES_TO_SECONDS 60
#define SECONDS_TO_MILLIS 1000
#define MAX_SCRIPT_SIZE   100 * 13
//modes
#define SAVE_SCRIPT_MODE 1 // mode pour l'enregistrement d'un nouveau script
#define PLAY_SCRIPT_MODE 2 // mode pour lire un script
#define IDLE_MODE        3 // mode pour ne rien faire

/* ------ Variable de gestion d'activation aléatoire ---------------- */
//time to activate module 1 in millis
unsigned long nextActivationModule1 = 0;
// Module 1 : minimum in seconds
unsigned int minTimeActModule1 = 1 * MINUTES_TO_SECONDS;
// Module 1 : maximum in seconds
unsigned int maxTimeActModule1 = 2 * MINUTES_TO_SECONDS;
//time to deactivate module 1 in millis
unsigned long nextDeactivationModule1 = 0;
// Module 1 : minimum in seconds
unsigned int minTimeDeactModule1 = 1 * MINUTES_TO_SECONDS;
// Module 1 : maximum in seconds
unsigned int maxTimeDeactModule1 = 2 * MINUTES_TO_SECONDS;
bool activatedModule1 = false;
/* ----------------------------------------------------------------- */

/* ----- Gestion script sur carte SD ------- */
char scriptFileName[] = "script1.txt";

int mode = SAVE_SCRIPT_MODE;
int scriptArr[MAX_SCRIPT_SIZE] = {0};
int scriptIndex = 0;
/* Nombre d'élément dans le tableau script */
int scriptCount = 0;
unsigned long lastAction = 0;
//Frequence en millisecondes
int frequence = 50;
/* ------------------------------- */

/* Broche CS de la carte SD */
const byte SDCARD_CS_PIN = 10; // TODO A remplacer suivant votre shield SD

boolean debug = false;

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
 // altSerial.begin(19200);
  Serial2.begin(19200);
  xbee.setSerial(Serial2);

  /* Initialisation du port SPI */
  pinMode(15, OUTPUT); // port SPI teensy
  //  SPI.setMOSI(pin);
//  SPI.setMISO(pin);
//  SPI.setSCK(pin);

  delay(2000);

  /* Initialisation de la carte SD */
  Serial.print(F("Init carte SD... "));
  if (!SD.begin(SDCARD_CS_PIN)) {
    error("Carte SD FAIL");
  }
  Serial.println(F("Carte SD OK"));

  //Init module 1 activtion time
 // setNextTime(setupTime, minTimeActModule1, maxTimeActModule1, nextActivationModule1);
  
   Serial.println("go");
}

void loop()
{
  unsigned long loopTime = millis();
//Serial.println("loop");
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
  
    
    if(mode == IDLE_MODE || mode == SAVE_SCRIPT_MODE) {
      //En mode idle et en mode enregistrement, faire bouger Grot
      potards();
      actionAll();
      
      saveInBuffer();
    }
  }

  //Code de test : simulation de données reçu depuis le xbee
   if(mode == SAVE_SCRIPT_MODE) {
    if(loopTime >= lastAction + frequence) {
      fakeXbee();
      potards();
      saveInBuffer();

      lastAction = loopTime;
    }
   }
  //Lecture du script
  if(mode == PLAY_SCRIPT_MODE) {
    playScript(loopTime);
  }
}

/* Simule la réception de données par xbee */
void fakeXbee()
{
  int val = 0;
  for (int i = 0; i < NB_CHANNEL; i++) {
    TxVal[i] = val++;
  }
  
}

/* Lecture des données reçu par le xbee */
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
}

/* Actionne les moteurs */
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

void loopModule(unsigned long loopTime,  unsigned int minActTime, unsigned int maxActTime, unsigned long resultActTime, unsigned int minDeactTime, unsigned int maxDeactTime, unsigned long resultDeactTime, bool activatedModule) {
   if (!activatedModule && loopTime >= nextActivationModule1) {
    // set deactivation time
    setNextTime(loopTime, minDeactTime, maxDeactTime, resultDeactTime);
 
    // activate module

    activatedModule = true;
  } else if (activatedModule1 && loopTime >= nextDeactivationModule1) {
    // set next activation time
    setNextTime(loopTime, minActTime, maxActTime, resultActTime);
    
    //deactivate module

    activatedModule = false;
  }
}

void setNextTime(unsigned long currentTime, unsigned int minTime, unsigned int maxTime, unsigned long resultTime) {
  resultTime = currentTime + getRandomTime(minTime, maxTime);
}

// minimum and maximum in seconds
unsigned long getRandomTime(unsigned int minimum, unsigned int maximum) {
  return random(minimum, maximum) * SECONDS_TO_MILLIS;
}

/* Enregistre les valeurs courrantes dans le buffer */
void saveInBuffer() {
   if(mode == SAVE_SCRIPT_MODE) {
//        Serial.print("scriptIndex ");
//        Serial.println(scriptIndex);
        //enregistrement des commandes
        // scriptArr[scriptIndex] = countCmd; //pour test
        scriptArr[scriptIndex] = rotation;
        scriptIndex++;
        scriptArr[scriptIndex] = s_cou_d;
        scriptIndex++;
        scriptArr[scriptIndex] = s_cou_g;
        scriptIndex++;
        scriptArr[scriptIndex] = s_mach;
        scriptIndex++;
        scriptArr[scriptIndex] = s_sou_g;
        scriptIndex++;
        scriptArr[scriptIndex] = s_sou_d;
        scriptIndex++;
        scriptArr[scriptIndex] = s_linf;
        scriptIndex++;
        scriptArr[scriptIndex] = s_lsup_g;
        scriptIndex++;
        scriptArr[scriptIndex] = s_lsup_d;
        scriptIndex++;
        scriptArr[scriptIndex] = s_oeil_g;
        scriptIndex++;
        scriptArr[scriptIndex] = s_oeil_d;
        scriptIndex++;
        scriptArr[scriptIndex] = s_arcin;
        scriptIndex++;
        scriptArr[scriptIndex] = s_arcex;
        scriptIndex++;
        
        //countCmd++; //pour test
  
        //Mettre automatiquement fin à l'enregistrement si le tableau est plein.
        //if(scriptIndex > 130) {
        if(scriptIndex > MAX_SCRIPT_SIZE) {
          Serial.println("fin enregistrement");
          Serial.println(scriptIndex);
          openFileAndWriteScript();
         
          //mettre le mode à IDLE_MODE pour ne pas lancer la lecture du script juste à la fin de l'enregistrement
          setMode(PLAY_SCRIPT_MODE);
        }
      }
}

void  playScript(unsigned long loopTime) {
  Serial.println(mode);
  Serial.println(scriptIndex);
  Serial.println(scriptCount);
  
  if(mode == PLAY_SCRIPT_MODE && scriptIndex < scriptCount && loopTime >= lastAction + frequence) {
    //Arrêt de la lecture si la suite du script est vide
    Serial.println(scriptArr[scriptIndex]);
    scriptIndex++;
    lastAction = loopTime;
    
  } 
  
  if(scriptIndex >= scriptCount) {
    Serial.println("Fin du script");
    //Mettre fin à la lecture
    setMode(IDLE_MODE);
  }
}

//Change de mode et remet les compteurs à 0.
void setMode(int newMode) {
  //Remettre les variables à 0
  scriptIndex = 0;
  lastAction = 0;

  if(newMode == PLAY_SCRIPT_MODE && mode != newMode) {
    loadScriptFromFile();
  }

  // Change le mode
  mode = newMode;
}

/** Fonction de chargement du fichier de script */
void loadScriptFromFile() {
  Serial.println(F("Chargement du script en cours ..."));
  File file = SD.open(scriptFileName, FILE_READ);
  if(!file) {
    // Erreur d'ouverture du fichier
    error("Impossible d'ouvrir le fichier");
  }

  uint16_t myInt;
  byte buf[2];
  while (file.available() > 0) {
    // Lit deux octets du fichier tant qu'il y a des données à lire
    buf[0] = file.read();
    buf[1] = file.read();
    myInt = buf[0] << 8 | buf[1];
    scriptArr[scriptIndex] = myInt;
    scriptIndex++;
  }
  scriptCount = scriptIndex - 1;
  scriptIndex = 0;
  file.close();
  
  Serial.println(F("Chargement du script terminé"));
}

/** Fonction d'écriture dans le fichier */
void openFileAndWriteScript(){
  Serial.println(F("Préparation du fichier script en cours ..."));
  deleteFile(scriptFileName);
  delay(500);
  scriptCount = 0;
  File file = SD.open(scriptFileName, FILE_WRITE);
  if(!file) {
    // Erreur d'ouverture du fichier
    error("Impossible d'ouvrir le fichier");
  }

  byte arr[2];
  for (int i = 0 ; i < scriptIndex ; i++) {
    arr[1] = scriptArr[i] & 0xff;
    arr[0] = (scriptArr[i] >> 8) & 0xff;
    if(file.write(arr, 2) == 0) {
       Serial.println(F("Erreur d'écriture dans le fichier"));
    }
  }

  file.close();
  Serial.println(F("Script enregistré"));
}

/**  Fonction de suppression d'un dossier avec gestion d'erreur */
void deleteFile(const char* filename) {
  if(SD.exists(filename)) {
    if(!SD.remove (filename)) {
      error("Erreur suppression fichier");
    }
  }
}

/** Log l'erreur et bloque le code */
void error(const char* message) {
  // Erreur d'ouverture du fichier
  Serial.println(F(message));
  for(;;); // Attend appui sur bouton RESET
}
