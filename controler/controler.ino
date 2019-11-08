#include <SPI.h>
#include <SD.h>

#define MINUTES_TO_SECONDS 60
#define SECONDS_TO_MILLIS 1000
#define MAX_SCRIPT_SIZE   3600
//modes
#define SAVE_SCRIPT_MODE 1 // mode pour l'enregistrement d'un nouveau script
#define PLAY_SCRIPT_MODE 2 // mode pour lire un script
#define IDLE_MODE        3 // mode pour ne rien faire


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

char scriptFileName[] = "script.txt";

int mode = SAVE_SCRIPT_MODE;
int scriptArr[MAX_SCRIPT_SIZE] = {0};
int scriptIndex = 0;
/* Nombre d'élément dans le tableau script */
int scriptCount = 0;
unsigned long lastAction = 0;

/* Broche CS de la carte SD */
const byte SDCARD_CS_PIN = 10; // TODO A remplacer suivant votre shield SD

//Frequence en millisecondes
int frequence = 50;

int countCmd;

void setup() {
  delay(100);
  unsigned long setupTime = millis();

/* Initialisation du port série (debug) */
  Serial.begin(115200);

  /* Initialisation du port SPI */
  pinMode(10, OUTPUT); //TODO port SPI teensy

  /* Initialisation de la carte SD */
  Serial.print(F("Init carte SD... "));
  if (!SD.begin(SDCARD_CS_PIN)) {
    error("Carte SD FAIL");
  }
  Serial.println(F("Carte SD OK"));

  //Init module 1 activtion time
  setNextTime(setupTime, minTimeActModule1, maxTimeActModule1, nextActivationModule1);
  
  Serial.println("fin setup");
}

void loop() {
  unsigned long loopTime = millis();

  //read mode : mode lecture ou mode ecriture
  // -- Lire le bouton et mettre la variable readMode à true pour enregistrer des commandes.
  //mettre readMode à false pour jouer la secende de commande.
 // isReadMode = true;

 //test interuption de l'enregistrement
// if(scriptCount > 100 && mode == SAVE_SCRIPT_MODE) {
//  Serial.println("test stop");
//  setMode(PLAY_SCRIPT_MODE);
// }

  if (mode == SAVE_SCRIPT_MODE) {
    if(loopTime >= lastAction + frequence) {
      // lecture de la commande (remplacer le countCmd par la vraie commande)
      scriptArr[scriptIndex] = countCmd;
      scriptIndex++;
      countCmd++;
      lastAction = loopTime;

      //Mettre automatiquement fin à l'enregistrement si le tableau est plein.
      if(scriptIndex > MAX_SCRIPT_SIZE) {
        Serial.println("fin enregistrement");
       openFileAndWriteScript();
       
        //mettre le mode à IDLE_MODE pour ne pas lancer la lecture du script juste à la fin de l'enregistrement
        setMode(PLAY_SCRIPT_MODE);
      }
    }
    
  } else if(mode == PLAY_SCRIPT_MODE) {
    //test de la fonction de lecture de script
    playScript(loopTime);
    
    //Module 1    
//    loopModule (loopTime,
//                minTimeActModule1, maxTimeActModule1, nextActivationModule1,
//                minTimeDeactModule1, maxTimeDeactModule1, nextDeactivationModule1,
//                activatedModule1);
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

void  playScript(unsigned long loopTime) {
  if(mode == PLAY_SCRIPT_MODE && scriptIndex < MAX_SCRIPT_SIZE && loopTime >= lastAction + frequence) {
    //Arrêt de la lecture si la suite du script est vide
    if(scriptArr[scriptIndex] != 0) { 
      Serial.println(scriptArr[scriptIndex]);
      scriptIndex++;
      lastAction = loopTime;
    }
  }
}

//Change de mode et remet les compteurs à 0.
void setMode(int newMode) {
  //Remettre les variables à 0
  scriptIndex = 0;
  lastAction = 0;
  scriptCount = 0;

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

  while (file.available() > 0) {
    // Lit un octet du fichier tant qu'il y a des données à lire
    scriptArr[scriptIndex] = file.read();
    scriptIndex++;
  }
  scriptCount = scriptIndex - 1;
  file.close();
  
  Serial.println(F("Chargement du script terminé"));
}

/** Fonction d'écriture dans le fichier */
void openFileAndWriteScript(){
  Serial.println(F("Préparation du fichier script en cours ..."));
  File file = SD.open(scriptFileName, FILE_WRITE);
  if(!file) {
    // Erreur d'ouverture du fichier
    error("Impossible d'ouvrir le fichier");
  }

  for (int i = 0 ; i < scriptIndex ; i++) {
    if(file.write((byte)scriptArr[i]) == 0) {
       Serial.println(F("Erreur d'écriture dans le fichier"));
    }
  }

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
