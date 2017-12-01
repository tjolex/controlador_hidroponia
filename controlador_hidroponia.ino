/*
 * RAFAEL LUIZ REHN
 * Engenharia de Computação - UFSM
 * Trabalho de Conclusão de Curso
 * Controlador para sistemas hidropoônicos
 * Medição do PH e condutividade elétrica dos líquidos
 * Controle dos relés para acionamento das bombas
 * 2017
 * rafaelluizrehn@gmail.com
 */
 
// Variavel para controle de tempo de medição
int timer=3600;

//Carrega a biblioteca LiquidCrystal
#include <LiquidCrystal.h>

//Carrega a biblioteca do Termistor NTC10k
#include <Thermistor.h>

Thermistor t(A4); //porta analogica para leitura dos dados do termisotor encapsulado;
//Thermistor t2(A4); //porta analogica para leitura dos dados do termisotor encapsulado;

//Define os pinos que serão utilizados para ligação ao display
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//VARIAVEIS PH E CONDUTIVIDADE ELETRICA
float phValue,C;

//Definição dos pinos de controle dos relés
int rele_1 = 26;
int rele_2 = 28;
int rele_3 = 30;


/*
  Condutivímetro 01
  Mede a tensão no ponto médio do divisor de tensão e converte em Volts
  Calcula a resistencia da solução a partir da tensão no ponto médio do divisor de tensão
*/
const float V_arduino = 5.00; //Tensão nos pinos digitais do Arduino
byte electrode_1 = 7; //Pino que servirá como eletrodo LOW
byte electrode_2 = 8; //Pino que servirá como eletrodo HIGH
int  reading_pin = A9; //Pino analógico para leitura

/*
 * Variáveis para detecção do parâmetro de PH de líquidos
 */
const int analogInPin = A0; //Pino que servi~á para porta de leitura
int sensorValue = 0; 
float b;

/*
 * Variáveis para controle do debaunce do botão
 */
const int  buttonPin = 38;    // Pino que o botao está ligado
const int ledPin = 13;       // Pino onde o led está ligado

int buttonPushCounter = 0;   // Conta o numero de vezes que o botão é pressionado
int buttonState = 0;         // Estado atual do botão
int lastButtonState = 0;     // Estado anterior do botão

//funcoes
void botao() {
  // Faz a leitura do momento em que o botão é pressionado
  buttonState = digitalRead(buttonPin);
  // Compara o estado do botão com o estado anterior
  if (buttonState != lastButtonState) {
    // Se o estado muda, incrementa contador
    if (buttonState == HIGH) {
      digitalWrite(ledPin, HIGH);
      buttonPushCounter++;
      timer=3600;
      Serial.print("Botão pressionado, contagem: ");
      Serial.println(buttonPushCounter);
      delay(1000);
      digitalWrite(ledPin, LOW);
    }
    // Adiciona um pequeno atraso para evitar o bounce
    delay(50);
  }
}

void f_temperatura(){
  int temperatura= t.getTemp();
  //Serial.print(" Temperatura: ");
  //Serial.print(temperatura);
  //Serial.println("*C");
  lcd.setCursor(7,0);
  lcd.print(temperatura);
}

void f_condutividade(){
  float Vm;   // Variável que irá armazenar as leituras em Volts no ponto médio do divisor de tensão
  float Rc = 10000; //Resistência constante
  float Rx; //Resistência da solução
  float C;
  unsigned long int avgValue;  
  int buf_c[10];
  int temp_c;
  int i,j;
 
  for(i=0;i<10;i++){
    analogWrite(electrode_2, 127 ); //Colocar eletrodo 2 como polo positivo 5V
    analogWrite(electrode_1, 0);   //Colocar eletrodo 1 como polo negativo 0V
    delay(50);                         //Aguarda estabilização (?)
    buf_c[i] = analogRead(reading_pin);  //Medir tensão entre o divisor de voltagem
    analogWrite(electrode_2, 0);     //Desliga eletrodo 1
  }
  // Realiza 10 medidas descarta as duas mais baixas e as duas mais altas
  // E faz a média das 6 restantes
    for(i=0;i<9;i++){
      for(j=i+1;j<10;j++){
        if(buf_c[i]>buf_c[j]){
          temp_c=buf_c[j];
          buf_c[i]=buf_c[j];
          buf_c[j]=temp_c;
        }
      }
     }
  avgValue=0;
  for(i=2;i<8;i++)
  avgValue+=buf_c[i];
  avgValue=avgValue/6;
    
  Vm = avgValue * 0.0048;    //Converte de ADC para Volts
  Rx = Rc * Vm / (V_arduino - Vm); //Calcula a resistência equivalente da solução
  Rx=(Rx);
  C=(1000000/Rx);
  Serial.print(" Tensão (V) = ");
  Serial.print(Vm);
  Serial.print("-> Resistencia (Kohms) = ");
  Serial.print(Rx);
  Serial.print("-> Condutividade (uS) = ");
  Serial.println(C);
//  Exibe os parâmetros lidos no display
  lcd.setCursor(11,1);
  lcd.print(C); 
  lcd.setCursor(2, 0);
  lcd.print(Vm);
  lcd.setCursor(2,1);
  lcd.print(Rx);
  //}
}

void f_ph(){
  unsigned long int avgValue; 
  int buf[10],temp;
  int i;
  int j;
  for(i=0;i<10;i++){ 
  buf[i]=analogRead(analogInPin);
  delay(10);
 }
 // Realiza 10 medidas descarta as duas mais baixas e as duas mais altas
 // E faz a média das 6 restantes
 for(i=0;i<9;i++){
  for(j=i+1;j<10;j++){
   if(buf[i]>buf[j]){
    temp=buf[i];
    buf[i]=buf[j];
    buf[j]=temp;
   }
  }
 }
 avgValue=0;
 for(i=2;i<8;i++)
 avgValue+=buf[i];
 float pHVol=(float)avgValue*5.0/1024/6;
 phValue = -5.7 * pHVol + 21.64;
 Serial.print(" PH = ");
 Serial.print(phValue);
 lcd.setCursor(12,0);
 lcd.print(phValue);
 delay(20);
}

void setup()
{
  // Define os sentidos de alguns pinos
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  
  pinMode(rele_1,OUTPUT);
  pinMode(rele_2,OUTPUT);
  pinMode(rele_3,OUTPUT);
  
  Serial.begin(9600);   
    //Define o número de colunas e linhas do LCD
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("V:");
  lcd.setCursor(0, 1);
  lcd.print("R:");
  lcd.setCursor(9,0);
  lcd.print("PH:");
  lcd.setCursor(8,1);
  lcd.print("C:");
  lcd.setCursor(6,0);
  lcd.print("T");
    
  pinMode(electrode_1, OUTPUT); 
  pinMode(electrode_2, OUTPUT);

  
}
 
void loop()
{
  int z;
  // Coloca em loop o botão, caso não seja ativado segue pra contagem automática
  botao();
  // Incia fazendo a primeira medida e então aguarda 1 hora ou até ser pressionado o botão
  // Quando o botão é pressionado altera a variavel timer para =-3600
  if(timer==3600){
    Serial.println(" INICIA PROGRAMA DE MEDIDAS: ");
    //faz 30 medidas e guarda variável do ultimo valor lido
     for(z=0;z<5;z++){
        Serial.print("Medida nº: ");
        Serial.print(z);
        digitalWrite(ledPin, HIGH);
        f_ph();
        f_condutividade();        
        delay(1000); //1s
     }
     Serial.println("Valores das variáveis ");
     Serial.print("pH: ");
     Serial.println(phValue);
     Serial.print("C: ");
     Serial.println(C);     
     // Verifica a ultima variavel armazenada e aciona as bombas caso necessário
     if(C<=1,5){
      Serial.println("Adicionando nutrientes, aguardando 90s...");
      digitalWrite(rele_1, LOW);
      delay(9000);
      digitalWrite(rele_1, HIGH);
     }
     if(phValue<6){
      Serial.println("Subindo pH, aguardando 30.73s...");
      digitalWrite(rele_2, LOW);
      delay(3773);
      digitalWrite(rele_2, HIGH);
     }
     if(phValue>7){
      Serial.println("Baixando pH, aguardando 9.58...");
      digitalWrite(rele_3, LOW);
      delay(958);
      digitalWrite(rele_3, HIGH);
     }
        
     digitalWrite(ledPin, LOW);
     timer=0;
  }else{
    timer++;
  }
    
    f_temperatura();
    Serial.print("Timer(s): ");
    Serial.println(timer);
  delay(1000); //1s

}
