#include <stdlib.h>
#include <string.h>

int Registre = 0;
int temps_mostra = 1;
bool Estat = false;
int Counter = 0;

void setup()
{
  Serial.begin(9600);
  Serial.setTimeout(50);

  // initialize timer1
  noInterrupts();                        // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 62499;                         // compare match register 16MHz/256/1Hz
  TCCR1B |= (1 << WGM12);                // CTC mode
  TCCR1B |= (1 << CS12);                 // 256 prescaler
  TIMSK1 |= (0 << OCIE1A);               // enable timer compare interrupt
  interrupts();                          // enable all interrupts
}

ISR(TIMER1_COMPA_vect)                   // timer compare interrupt service routine
{
  Counter++;
  if (Counter == temps_mostra) {       //Condicio del compliment del temps de mostra
    Registre = analogRead(A0);         //Lectura analogica
    Counter = 0;
  }
}

void loop()
{
  String Comanda = "";
  String Resposta = "";
  if (Serial.available())                                     //Interrupcio del port serie
  {
    char data[10];
    for ( int i = 0; i < sizeof(data);  ++i )                 //Inicialitzacio del array de caracters
      data[i] = (char)0;
    size_t count = Serial.readBytesUntil('\n', data, 20);     //Lectura del port serie
    Comanda = data;
    Resposta = Error_Protocol(Comanda);                       //Crida de subrutina
    for ( int k = 0; k <= sizeof(Resposta) + 1;  ++k )        //String to Array Char
      data[k] = Resposta[k];
    Serial.write(data);                                       //Escriure per el bus serie
  }
}

String Error_Protocol(String Comanda)                         //Subrutina per l'error de protocol
{
  String Resposta;
  bool Error = false;
  if ((Comanda[0] == 'A') && (Comanda[Comanda.length() - 1] == 'Z')) { //Condicio per els caracter d'inici de trama i final de trama
    for (int i = 1; i <= Comanda.length(); i++) {                      //Comprovacio per no rebre dos caracters inici de trama, sense un caracter final de trama
      if (Comanda[i] == 'A') {
        Error = true;
        break;
      }
    }
  }
  else {
    Error = true;
    Resposta = "AM1Z";
  }
  if (Error == false) {
    Resposta = Sel_Accio(Comanda);                                     //Crida subrutina
  }
  return Resposta;
}

String Sel_Accio(String Comanda)                                                      //Subrutina de seleccio de accio
{
  String Resposta;
  switch (Comanda[1]) {
    case 'M':                                                         //Marcha-Paro
      Resposta = Marcha_Paro(Comanda);
      break;
    case 'S':                                                         //Control de Sortides
      Resposta = AD_Sortides(Comanda);
      break;
    case 'E':
      Resposta = AD_Entrades(Comanda);                                                 //Lectura de Entrades
      break;
    case 'C':
      Resposta = Ultima_MostraADC();                                            //Enviar ultima adquisicio
      break;
    default:
      Resposta = "AM1Z";
      break;
  }
  return Resposta;
}

String Marcha_Paro(String Comanda)                                                    //Subrutina Marcha-Paro
{
  String Resposta;
  if ((Comanda[2] == '1') && (Estat == false)) {                      //Comprovacio d'estat anterior
    temps_mostra = Comanda.substring(3, 5).toInt();                   //Adquirir temps de mostra
    if (temps_mostra <= 20) {                                         //Temps dins dels parametres 1-20s
      Estat = true;                                                   //Iniciar maquina
      TIMSK1 |= (1 << OCIE1A);                                        // disable timer compare interrupt
      Resposta = "AM0Z";
    }
    else {
      Resposta = "AM2Z";
    }
  }
  else if ((Comanda[2] == '0') && (Estat == true)) {                  //Comprovacio d'estat anterior
    Estat = false;                                                    //Parar maquina
    TIMSK1 &= ~(1 << OCIE1A);                                         // disable timer compare interrupt
    Resposta = "AM0Z";
  }
  else {
    Resposta = "AM2Z";
  }
  return Resposta;
}

String AD_Sortides(String Comanda)                                                    //Subrutina Control de Sortides
{
  String Resposta;
  int n_pin = 0;
  int v = 0;
  n_pin = Comanda.substring(2, 4).toInt();                            //Adquirir numero de pin
  v = Comanda.substring(4).toInt();                                   //Indicar estat de sortida
  if ((2 <= n_pin) && (n_pin <= 13)) {                                //Comprovacio de parametres pins (2-13)
    if ((v == 0) || (v == 1)) {                                       //Comprovacio de estat dels pins (0-1)
      pinMode(n_pin, OUTPUT);                                         //Configurar pin
      digitalWrite(n_pin, v);                                         //Iniciar pin
      Resposta = "AS0Z";
    }
    else {
      Resposta = "AS2Z";
    }
  }
  else {
    Resposta = "AS2Z";
  }
  return Resposta;
}
String AD_Entrades(String Comanda)                                                    //Subrutina Lectura de Entrades
{
  String Resposta;
  int n_pin = 0;
  int v = 0;
  n_pin = Comanda.substring(2, 4).toInt();                            //Adquirir numero de pin
  v = Comanda.substring(4).toInt();                                   //Adquirir estat del pin
  if ((2 <= n_pin) && (n_pin <= 13)) {                                //Comprovacio de parametres pins (2-13)
    pinMode(n_pin, INPUT);                                            //Configurar pin
    v = digitalRead(n_pin);                                           //Lectura del pin
    Resposta = "AE0" + (String)v + "Z";                               //Construccio de la trama
  }
  else {
    Resposta = "AE20Z";
  }
  return Resposta;
}
String Ultima_MostraADC()                                               //Subrutina Enviar ultima adquisicio
{
  String Resposta;
  int Mostra = Registre;
  String Mostra_String = (String)Mostra;                              //Int to String
  int resta = 4 - Mostra_String.length();
  for (int i = 0; i < resta ; i++) {                                  //Completar trama amb 0
    Mostra_String = 0 + Mostra_String;
  }
  Resposta = "AC0" + Mostra_String + "Z";
  return Resposta;
}
