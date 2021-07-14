/* Example sketch to control a stepper motor with TB6600 stepper motor driver and Arduino without a library: number of revolutions, speed and direction. More info: https://www.makerguides.com */
// Define stepper motor connections and steps per revolution:
#define microStep 8
#define dirPinR 2
#define stepPinR 3
#define dirPinM 5
#define stepPinM 6
#define endstopPin 12
#define stepsPerRevR 800
#define stepsPerRevM 3200
#define leadScrewStep 2.5
int sign = 1;
int beginRevolutions = 3;
int endRevolutions = 3;
float windStep = 2.0;
float windLength = 40.0;

enum winderState
{
  HOMING,
  BEGIN,
  WIND,
  END,
  IDLE
};
winderState state = HOMING;

void setup()
{
  //Задаем входной пин для концевика и поддтягиваем этот пин
  pinMode(endstopPin, INPUT_PULLUP);
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

  switch (winderState)
  {
  case HOMING:
    homeCarriage();
    winderState = BEGIN;
    break;
  case BEGIN:
    windBegin();
    winderState = WIND;
    break;
  case WIND:
    windWire();
    winderState = END;
    break;
  case END:
    windEnd();
    winderState = IDLE;
    break;
  }
}

void checkSerial()
{
  if (Serail.available() > 0)
  {
    String cmd = Serail.readStringUntil('\n');
    if (cmd == "Start")
    {
      winderState = HOMING;
      return;
    }
    char cmdID = cmd.charAt(0);
    int value = cmd.substring(1);
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
  makeRevolutions(beginRevolutions);
}

void windEnd()
{
  makeRevolutions(endRevolutions);
}

void makeRevolutions(int revolutions)
{
  for (int i = 0; i < stepsPerRevM * revolutions; i++)
  {
    digitalWrite(stepPinR, HIGH);
    delayMicroseconds(60);
    digitalWrite(stepPinR, LOW);
    delayMicroseconds(420);
  }
}

void windWire()
{
  float step = 0.01;
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