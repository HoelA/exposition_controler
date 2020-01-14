#include <SPI.h>
#include <SD.h>

#define MAX_SCRIPT_SIZE  10
#define MAX_INT 65535

/* Broche CS de la carte SD */
const byte SDCARD_CS_PIN = 10; // TODO A remplacer suivant votre shield SD

File file;
char scriptFileName[] = "sc10.txt";

void setup() {
  Serial.begin(9600);
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

}

bool isSave = false;


void loop() {
  // put your main code here, to run repeatedly:

  if (isSave) {
    //save
    file = SD.open(scriptFileName, FILE_WRITE);
    for (int i = 0; i < MAX_SCRIPT_SIZE; i++) {
      saveValueInFile(i);
    }
    saveValueInFile(MAX_INT);
    file.close();
    isSave = false;
  } else {
    //read
    file = SD.open(scriptFileName, FILE_READ);

    uint16_t myInt;
    while ((myInt=readValueFromFile()) != MAX_INT) {
      Serial.println(myInt);
    }

    file.close();
    error("Fin du test");
  }
}

void saveValueInFile(int value) {
  Serial.print("saveValueInFile ");
  Serial.println(value);
  byte arr[2];
  arr[1] = value & 0xff;
  arr[0] = (value >> 8) & 0xff;
  if (file.write(arr, 2) == 0) {
    Serial.println(F("Erreur d'écriture dans le fichier"));
  }
}

uint16_t readValueFromFile() {
  uint16_t myInt = MAX_INT;
  byte buf[2];
  if (file.available() >= 2) {
    // Lit deux octets du fichier tant qu'il y a des données à lire
    buf[0] = file.read();
    buf[1] = file.read();
    myInt = buf[0] << 8 | buf[1];

    Serial.print("value = ");
    Serial.println(myInt);
  }

  return myInt;
}

/** Log l'erreur et bloque le code */
void error(const char* message) {
  // Erreur d'ouverture du fichier
  Serial.println(F(message));
  for (;;); // Attend appui sur bouton RESET
}
