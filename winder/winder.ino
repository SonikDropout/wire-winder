/* Example sketch to control a stepper motor with TB6600 stepper motor driver and Arduino without a library: number of revolutions, speed and direction. More info: https://www.makerguides.com */
// Define stepper motor connections and steps per revolution:
#define microStep 8
#define dirPinR 2
#define stepPinR 3
#define dirPinM 5
#define stepPinM 6
#define endstopPin 12
#define buttonPin 10
#define stepsPerRevR 800
#define stepsPerRevM 800
#define leadScrewStep 2.5
int sign = 1;
int beginRevolutions = 3;
int endRevolutions = 3;
float windStep = 2.5;
float windLength = 25.0;

enum winderState
{
  HOMING,
  BEGIN,
  WIND,
  END,
  PAUSED,
  PAUSED_HOME
};
winderState state = PAUSED;

void setup()
{
  //Задаем входной пин для концевика и поддтягиваем этот пин
  pinMode(endstopPin, INPUT_PULLUP);
  pinMode(buttonPin, INPUT_PULLUP);
  // Задаем пины для управления моторами:
  pinMode(stepPinR, OUTPUT);
  pinMode(dirPinR, OUTPUT);
  pinMode(stepPinM, OUTPUT);
  pinMode(dirPinM, OUTPUT);

  Serial.begin(115200);
}

void loop()
{
  checkSerial();

  bool buttonPressed = !(digitalRead(buttonPin));

  switch (state)
  {
    case HOMING:
      homeCarriage();
      state = PAUSED_HOME;
      break;
    case BEGIN:
      windBegin();
      state = WIND;
      break;
    case WIND:
      windWire();
      state = END;
      break;
    case END:
      windEnd();
      state = PAUSED;
      break;
    case PAUSED:
      if (buttonPressed) {
        state = HOMING;
        break;
      }
    case PAUSED_HOME:
      if (buttonPressed) {
        state = BEGIN;
        break;
      }
  }
}

void checkSerial()
{
  if (Serial.available() > 0)
  {
    String cmd = Serial.readStringUntil('\n');
    char cmdID = cmd.charAt(0);
    String value = cmd.substring(1);
    switch (cmdID)
    {
      case 'S':
        windStep = value.toFloat();
        break;
      case 'L':
        windLength = value.toFloat();
        break;
      case 'B':
        beginRevolutions = value.toInt();
      case 'E':
        endRevolutions = value.toInt();
        break;
      default:
        Serial.write('ERROR');
        break;
    }
  }
}

void homeCarriage()
{
  sign = digitalRead(endstopPin);
  //Задаем для каретки левое направление движения
  digitalWrite(dirPinM, LOW);
  //Едем к концевику
  while (sign != 0)
  {
    digitalWrite(stepPinM, HIGH);
    delayMicroseconds(60);
    digitalWrite(stepPinM, LOW);
    delayMicroseconds(320);
    sign = digitalRead(endstopPin);
  }
}

void windBegin()
{
  // Задаем для шпинделя направление вращения по часовой
  digitalWrite(dirPinR, LOW);
  //Задаем для каретки правое направление движения
  digitalWrite(dirPinM, HIGH);
  // Делаем затравочные 3 витка на месте:
  rotateShaft(beginRevolutions);
}

void windEnd()
{
  rotateShaft(endRevolutions);
}

void windWire()
{
  float step = 0.1;
  float revolutionsPerStep = step / windStep;

  float moved = 0.0;
  // Делаем проход
  while (moved < windLength) {
    moveCarriage(step);
    rotateShaft(revolutionsPerStep);
    moved += step;
  }
}

void moveCarriage(float distance) {
  int steps = distance / leadScrewStep * stepsPerRevM;
  for (int i = 0; i < steps; ++i)
  {
    digitalWrite(stepPinM, HIGH);
    delayMicroseconds(60);
    digitalWrite(stepPinM, LOW);
    delayMicroseconds(320);
  }
}

void rotateShaft(float revolutions) {
  int steps = revolutions * stepsPerRevR;
  for (int i = 0; i < steps; ++i)
  {
    digitalWrite(stepPinR, HIGH);
    delayMicroseconds(60);
    digitalWrite(stepPinR, LOW);
    delayMicroseconds(320);
  }
}
