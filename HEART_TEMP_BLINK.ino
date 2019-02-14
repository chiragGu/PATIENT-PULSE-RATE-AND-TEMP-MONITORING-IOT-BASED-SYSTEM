#include <OneWire.h>

#include <DallasTemperature.h>

#define LED 13                                                              // LED IS PINNED TO PIN 13

#define sensor_con 2                                                       // TEMPERATURE SENSOR IS CONNECTED TO PIN 2                             
    
    OneWire x(sensor_con);                                                 // AN OBJECT OF ONEWIRE IS MADE             
    
    DallasTemperature sensor(&x);                                         // THE OBJECT OF ONEWIRE IS REFERENCED IN THE DALLASTEMPERATURE OBJECT           
 
int pulsePin = 0;                                                         // HEARTBEAT SENSOR IS PINNED TO ANALOG INPUT A0
 
int blinkPin = 13;                  

volatile int BPM;          

volatile int Signal;              
 
volatile int IBI = 600;                                                 //IBI= INTER BEAT INTERVAL

volatile boolean Pulse = false;    

volatile int rate[10];                           

volatile int P = 512;          

volatile int T = 512;        

volatile int thresh = 512;              

volatile int amp = 100;                 

volatile boolean firstBeat = true;      

volatile boolean secondBeat = false;    

volatile unsigned long samplecounter = 0;  
volatile unsigned long lastBeatTime = 0;



void setup()

{

  pinMode(blinkPin,OUTPUT);        
  //  Serial.begin(9600);       
  //  Serial.println("Temperature Demo ");

   sensor.begin();                                    
     pinMode(LED, OUTPUT);
                     
  Serial.begin(115200);                                               // BAUD RATE OVER WHICH BOTH TEMPERATURE AND HEARTBEAT SENSOR WOULD OPERATE

  interruptSetup();                  

}



void loop()

{ 
digitalWrite(LED, HIGH);   
  
  delay(50);                                                        //DELAY OF 50ms
  
  digitalWrite(LED , LOW);    
  
  delay(500);                     
  
  Serial.print("Requesting Temperature:");

  sensor.requestTemperatures();                                     // INBUILT FUNCTION FOR TAKING THE INPUT OF THE TEMPERATURE               

  Serial.println("Input successful");
  
  Serial.print("Temperature Readings are:");
    
  Serial.println(sensor.getTempFByIndex(0));                        // FUNCTION TO RETURN RECORDED TEMPERATURE IN FAHRENHEIT
                                                    
  
      Serial.print("BPM: ");

      Serial.println(BPM);

      delay(200);                                                   // DELAY FOR 200 ms
  
}



void interruptSetup()

{    

  TCCR2A = 0x02;                                                     // THESE ARE TIMER2 REGISTERS REQUIRED FOR A CONSTANT INPUT OF HEARTBEAT AFTER 2 ms CONTINOUSLY. 
  OCR2A  = 0X7C;   
  TCCR2B = 0x06;  
  TIMSK2 = 0x02;  

  sei();                                                            // GLOBAL INTERUPTS ARE ENABLED

}


ISR(TIMER2_COMPA_vect)

{ 

  cli();                                                            // GLOBAL INTERRUPTS ARE DISABLED                               

  Signal = analogRead(pulsePin);                                    // TAKES THE INPUT FROM THE ANALOG PIN A0     
  
  samplecounter += 2;                        

  int N = samplecounter - lastBeatTime;      


  if(Signal < thresh && N > (IBI/5)*3)                             // KEEPING THE TRACK OF THE PEAK AND TROUGH OF THE WAVE

    {     

      if (Signal < T)

      {                       

        T = Signal;

      }

    }


  if(Signal > thresh && Signal > P) 

    {        

      P = Signal;

    }                          



   if (N > 250)                                                      // CHECKING IF WE HAVE A PULSE 

  {                            

    if ( (Signal > thresh) && (Pulse == false) && (N > (IBI/5)*3) )

      {       

        Pulse = true;          

        digitalWrite(blinkPin,HIGH);

        IBI = samplecounter - lastBeatTime;

        lastBeatTime = samplecounter;     



       if(secondBeat)                                             // USED TO GET REALISTIC AND ACCURATE READINGS 

        {                        

          secondBeat = false;   

          for(int i=0; i<=9; i++)    

          {            

            rate[i] = IBI; //Filling the array with the heart rate values                    

          }

        }


        if(firstBeat)                                            // FIRST READING IS DISCARDED AS IT IS CONSIDERED LOUSY I.E. NOT VERY ACCURATE

        {                        

          firstBeat = false;

          secondBeat = true;

          sei();            

          return;           

        }  


      word runningTotal = 0;                                    // CALCULATION OF BPM


      for(int i=0; i<=8; i++)

        {               

          rate[i] = rate[i+1];

          runningTotal += rate[i];

        }


      rate[9] = IBI;             

      runningTotal += rate[9];   

      runningTotal /= 10;        

      BPM = 60000/runningTotal;

    }                      

  }




  if (Signal < thresh && Pulse == true)                         // DETECTION OF PULSE THAT IS DECLINING 
                                                                // ALSO CALLED NON-BEAT
    {  

      digitalWrite(blinkPin,LOW); 

      Pulse = false;             

      amp = P - T;

      thresh = amp/2 + T; 

      P = thresh;           

      T = thresh;

    }


  if (N > 2500)                                               // WHEN THERE IS NO BEAT TO DETECT

    {                          

      thresh = 512;                     

      P = 512;                 

      T = 512;               

      lastBeatTime = samplecounter;     

      firstBeat = true;                 

      secondBeat = false;               

    }


 sei();  
}                              
