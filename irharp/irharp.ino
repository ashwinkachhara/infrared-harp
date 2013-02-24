#include<avr/io.h>
#include<avr/interrupt.h>
#include<math.h>
#include<inttypes.h>
 
#define SAMPLE 64
#define TONES 3
#define countTenMS 78
#define FULL 31
#define ALLSTRINGS 6
 
int count,waveNum,i;
 
unsigned char envtable[TONES][SAMPLE];
unsigned char vibetable[SAMPLE];
unsigned char pianotable[SAMPLE];
unsigned char organtable[SAMPLE];
 
const long incTable[14]={ 228313667L,//G#
			  241892640L,// A (440 Hz)
			  271524488L,// B
			  304784726L,// C#
			  322871699L,// D
			  362454131L,// E
			  406819440L,// F#
			  456627334L,// G#
			  483785280L,// A (880 Hz)
			  543048977L,// B
			  609514477L,// C#
			  645798373L,// D
			  724853286L,// E
			  0L 	   // default, kills string
			  };
 
unsigned long assignedNote[ALLSTRINGS];
unsigned char trigger=0;
unsigned long accumulator;
volatile unsigned long highBits;
volatile unsigned long increment;
volatile unsigned long hourglass;
 
ISR(TIMER2_OVF_vect){
  if(trigger){     
    accumulator+=increment;
    highBits=(char) (accumulator>>26);
 
    switch(waveNum){
      case 0:  PORTB=((vibetable[highBits])*envtable[0][hourglass])>>6;
               break;
      case 1:  PORTB=((organtable[highBits])*envtable[1][hourglass])>>6; 
               break;
      case 2:  PORTB=((pianotable[highBits])*envtable[2][hourglass])>>6; 
               break;
    }
 
    count++;
 
    if(count==countTenMS){
      count=0;
      if(hourglass<FULL) hourglass++;
      else {
        trigger=0;
        digitalWrite(2,HIGH);
    }
  }
}
 
  if((PINC & 0b0111111)&& trigger==0){
    trigger=1;
 
    for(i=0;i<6;i++){
      if(PINC & (1<<i)){
        increment=assignedNote[i];
      }
    }
 
    digitalWrite(2,LOW);
    hourglass=0;
  }
}
 
void initialise(){
 
  for(i=0;i<SAMPLE;i++){
     vibetable[i]= (32+4*sin(i*6.28/(SAMPLE)*2)+4*sin(i*6.28/(SAMPLE)/15)+15*sin(i*6.28/(SAMPLE))+ 8*sin(i*6.28/(SAMPLE)*6))/1;
     organtable[i]= (32+4*sin(i*6.28/(SAMPLE)*6)+3*sin(i*6.28/(SAMPLE)/5)+15*sin(i*6.28/(SAMPLE))+ 3*sin(i*6.28/(SAMPLE)*4)+4*sin(i*6.28/(SAMPLE)/10)+2*sin(i*6.28/(SAMPLE)*2))/1;
     pianotable[i]= (32+5*sin(i*6.28/(SAMPLE)*2)+7*sin(i*6.28/(SAMPLE)/15)+17*sin(i*6.28/(SAMPLE))+ 2*sin(i*6.28/(SAMPLE)*4))/1;
  }
 
  for(i=0;i<SAMPLE;i++){
    if(i<12) envtable[0][i]=(255-10*i)/4;
    else if(i<44) envtable[0][i]=46-i;
    else envtable[0][i]=0;
  }
 
  for(i=0;i<SAMPLE;i++){
    if(i<15) envtable[1][i]=4.0*i;
    else if(i<54) envtable[1][i]=60+3*sin(i*6.28/(SAMPLE)*4);
    else if(i<64) envtable[1][i]=394-6.0*i;
    else envtable[1][i]=0;
  }
 
  for(i=0;i<SAMPLE;i++){
    if(i<21) envtable[2][i]=(255-10*i)/4;
    else if(i<44) envtable[2][i]=13;
    else envtable[2][i]=0;
  }
 
  count=0;
  waveNum=1;
  trigger=0;
  accumulator=0L;
  highBits=0;
  increment=0L;
  hourglass=FULL;
 
  assignedNote[0]=incTable[11];
  assignedNote[1]=incTable[9];
  assignedNote[2]=incTable[7];
  assignedNote[3]=incTable[5];
  assignedNote[4]=incTable[3];
  assignedNote[5]=incTable[1];
 
}
 
int main(){
  pinMode(2,OUTPUT);
  digitalWrite(2,HIGH);
  Serial.begin(9600);
  initialise();
 
  DDRB=0xFF;
  DDRC=0x00;
  TCCR2A=0b00000000;
  TCCR2B=0b00000010;
  TIMSK2=0b00000001;
  TCNT2=0;
  OCR2A=255;
  sei();
 
  while(1){}
  return 0;
}
