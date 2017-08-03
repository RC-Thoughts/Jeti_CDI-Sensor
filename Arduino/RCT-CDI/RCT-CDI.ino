/*
    --------------------------------------------------------
        Jeti Arduino CDI RPM Sensor v 1.0
    --------------------------------------------------------

      Tero Salminen RC-Thoughts.com 2017 www.rc-thoughts.com

      Very simple, RPM Monitor available on Jetibox.

      Hardware:
      - Arduino Pro Mini 3.3V 8Mhz

    --------------------------------------------------------
      ALWAYS test functions thoroughly before use!
    --------------------------------------------------------
      Shared under MIT-license by Tero Salminen 2017
    --------------------------------------------------------
*/

String sensVersion = "v.1.0";

// No settings to be defined by user
#include <SoftwareSerialJeti.h>
#include <JETI_EX_SENSOR.h>

unsigned int rpm;
int cdi = 5;
int rps;
long interval;
int timeset = 0;
int header = 0;
int lastbtn = 240;
int current_screen = 0;
int current_config = 0;
char temp[LCDMaxPos / 2];
char msg_line1[LCDMaxPos / 2];
char msg_line2[LCDMaxPos / 2];

#define prog_char char PROGMEM
#define GETCHAR_TIMEOUT_ms 20

#ifndef JETI_RX
#define JETI_RX 3
#endif

#ifndef JETI_TX
#define JETI_TX 4
#endif

#define ITEMNAME_1 F("RPM")
#define ITEMTYPE_1 F("rpm")
#define ITEMVAL_1 (unsigned int*)&rpm

#define ABOUT_1 F(" RCT Jeti Tools")
#define ABOUT_2 F(" CDI RPM-Sensor")

SoftwareSerial JetiSerial(JETI_RX, JETI_TX);

void JetiUartInit()
{
  JetiSerial.begin(9700);
}

void JetiTransmitByte(unsigned char data, boolean setBit9)
{
  JetiSerial.set9bit = setBit9;
  JetiSerial.write(data);
  JetiSerial.set9bit = 0;
}

unsigned char JetiGetChar(void)
{
  unsigned long time = millis();
  while ( JetiSerial.available()  == 0 )
  {
    if (millis() - time >  GETCHAR_TIMEOUT_ms)
      return 0;
  }
  int read = -1;
  if (JetiSerial.available() > 0 )
  { read = JetiSerial.read();
  }
  long wait = (millis() - time) - GETCHAR_TIMEOUT_ms;
  if (wait > 0)
    delay(wait);
  return read;
}

char * floatToString(char * outstr, float value, int places, int minwidth = 0) {
  int digit;
  float tens = 0.1;
  int tenscount = 0;
  int i;
  float tempfloat = value;
  int c = 0;
  int charcount = 1;
  int extra = 0;
  float d = 0.5;
  if (value < 0)
    d *= -1.0;
  for (i = 0; i < places; i++)
    d /= 10.0;
  tempfloat +=  d;
  if (value < 0)
    tempfloat *= -1.0;
  while ((tens * 10.0) <= tempfloat) {
    tens *= 10.0;
    tenscount += 1;
  }
  if (tenscount > 0)
    charcount += tenscount;
  else
    charcount += 1;
  if (value < 0)
    charcount += 1;
  charcount += 1 + places;
  minwidth += 1;
  if (minwidth > charcount) {
    extra = minwidth - charcount;
    charcount = minwidth;
  }
  if (value < 0)
    outstr[c++] = '-';
  if (tenscount == 0)
    outstr[c++] = '0';
  for (i = 0; i < tenscount; i++) {
    digit = (int) (tempfloat / tens);
    itoa(digit, &outstr[c++], 10);
    tempfloat = tempfloat - ((float)digit * tens);
    tens /= 10.0;
  }
  if (places > 0)
    outstr[c++] = '.';
  for (i = 0; i < places; i++) {
    tempfloat *= 10.0;
    digit = (int) tempfloat;
    itoa(digit, &outstr[c++], 10);
    tempfloat = tempfloat - (float) digit;
  }
  if (extra > 0 ) {
    for (int i = 0; i < extra; i++) {
      outstr[c++] = ' ';
    }
  }
  outstr[c++] = '\0';
  return outstr;
}

JETI_Box_class JB;

unsigned char SendFrame()
{
  boolean bit9 = false;
  for (int i = 0 ; i < JB.frameSize ; i++ )
  {
    if (i == 0)
      bit9 = false;
    else if (i == JB.frameSize - 1)
      bit9 = false;
    else if (i == JB.middle_bit9)
      bit9 = false;
    else
      bit9 = true;
    JetiTransmitByte(JB.frame[i], bit9);
  }
}

unsigned char DisplayFrame()
{
  for (int i = 0 ; i < JB.frameSize ; i++ )
  {
  }
}

#define MAX_SCREEN 2
#define MAX_CONFIG 1
#define COND_LES_EQUAL 1
#define COND_MORE_EQUAL 2

// setup functions
void setup()
{
  Serial.begin(9600);
  pinMode(cdi, INPUT_PULLUP);
  Serial.println("");
  Serial.print("RC-Thoughts CDI RPM-Sensor "); Serial.println(sensVersion);
  Serial.println("design by Tero Salminen @ RC-Thoughts 2017");
  Serial.println("Free and open-source - Released under MIT-license");
  Serial.println("");
  Serial.println("Ready!");
  Serial.println("");
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  pinMode(JETI_RX, OUTPUT);
  JetiUartInit();
  JB.JetiBox(ABOUT_1, ABOUT_2);
  JB.Init(F("RCT CDI"));
  JB.addData(ITEMNAME_1, ITEMTYPE_1);
  JB.setValueBig(1, ITEMVAL_1);
  do {
    JB.createFrame(1);
    SendFrame();
    delay(GETCHAR_TIMEOUT_ms);
  }
  while (sensorFrameName != 0);
  digitalWrite(13, LOW);
}

void process_screens()
{
  switch (current_screen)
  {
    case 0: {
      JB.JetiBox(ABOUT_1, ABOUT_2);
        break;
    }
    case 1: {
        msg_line1[0] = 0; msg_line2[0] = 0;
        strcat_P((char*)&msg_line1, (prog_char*)F("  RCT CDI RPM"));
        strcat((char*)&msg_line1, (char*)&temp);
        strcat_P((char*)&msg_line2, (prog_char*)F("RPM:"));
        temp[0] = 0;
        floatToString((char*)&temp, rpm, 0);
        strcat((char*)&msg_line2, (char*)&temp);
        JB.JetiBox((char*)&msg_line1, (char*)&msg_line2);
        break;
      }
    case MAX_SCREEN : {
        JB.JetiBox(ABOUT_1, ABOUT_2);
        break;
      }
  }
}

// Working loop
void loop()
{
  // Jeti Stuff
  unsigned long time = millis();
  SendFrame();
  time = millis();
  int read = 0;
  pinMode(JETI_RX, INPUT);
  pinMode(JETI_TX, INPUT_PULLUP);

  JetiSerial.listen();
  JetiSerial.flush();

  while ( JetiSerial.available()  == 0 )
  {
    if (millis() - time >  5)
      break;
  }

  if (JetiSerial.available() > 0 )
  { read = JetiSerial.read();

    if (lastbtn != read)
    {
      lastbtn = read;
      switch (read)
      {
        case 224 : // RIGHT
          current_screen++;
          break;
        case 112 : // LEFT
          current_screen--;
          break;
        case 208 : // UP
          break;
        case 176 : // DOWN
          break;
        case 144 : // UP+DOWN
          break;
        case 96 : // LEFT+RIGHT
          break;
      }
    }
  }

  if (current_screen != MAX_SCREEN)
    current_config = 0;
  process_screens();
  header++;
  if (header >= 5)
  {
    JB.createFrame(1);
    header = 0;
  }
  else
  {
    JB.createFrame(0);
  }

  long wait = GETCHAR_TIMEOUT_ms;
  long milli = millis() - time;
  if (milli > wait)
    wait = 0;
  else
    wait = wait - milli;
  pinMode(JETI_TX, OUTPUT);

  // RPM-Stuff
  // Wait 15 seconds after init to get sensor-value to receiver
  if (timeset == 0) {
    timeset = (time + 15000);
  }
  if (timeset < time) {
    interval = pulseIn(cdi, HIGH) + pulseIn(cdi, LOW);
    rps = 1000000UL / interval;
    if (rps == -1) {
      rpm = 0;
    }
    else {
      rpm = (rps * 60);
    }
  }
}

