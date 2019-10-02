#define MINUTES_TO_SECONDS 60
#define SECONDS_TO_MILLIS 1000

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

void setup() {
  unsigned long setupTime = millis();

  //Init module 1 activtion time
  setNextTime(setupTime, minTimeActModule1, maxTimeActModule1, nextActivationModule1);
  

}

void loop() {
  unsigned long loopTime = millis();

  //Module 1
  loopModule (loopTime,
              minTimeActModule1, maxTimeActModule1, nextActivationModule1,
              minTimeDeactModule1, maxTimeDeactModule1, nextDeactivationModule1,
              activatedModule1);
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

