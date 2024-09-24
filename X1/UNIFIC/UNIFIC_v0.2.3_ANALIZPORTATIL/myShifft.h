#define MAIN_SHIFFTER

#define BIT_Q0  0 
#define BIT_Q1  1
#define BIT_Q2  2
#define BIT_Q3  3 
#define BIT_Q4  4
#define BIT_Q5  5
#define BIT_Q6  6
#define BIT_Q7  7


class myShifft{
  private:
    byte status,prevStatus;
    byte initValue;
    int latchPin,clockPin,dataPin;

  public:
    myShifft(int latch,int clock, int data,byte initVal = 0x00){
      this->latchPin = latch;
      this->clockPin = clock;
      this->dataPin = data;
      pinMode(this->latchPin,OUTPUT);
      pinMode(this->clockPin,OUTPUT);
      pinMode(this->dataPin,OUTPUT);
      digitalWrite(this->latchPin, HIGH);
      this->status = initVal;
      this->initValue = initVal;
    }
    void set(int localBit){
      digitalWrite(latchPin, LOW);
      this->status |= (1<<localBit);
      shiftOut(dataPin, clockPin, MSBFIRST, status);
      digitalWrite(latchPin, HIGH);      
    }
    void clear(int localBit){
      digitalWrite(latchPin, LOW);
      this->status &= ~(1 << localBit);
      shiftOut(dataPin, clockPin, MSBFIRST, status);
      digitalWrite(latchPin, HIGH);      
    }
    byte getStatus(){
      return this->status;
    }
    void recStatus(){
      this->prevStatus = this->status;
    }
    void setPrevStatus(){
      this->status = this->prevStatus;
      digitalWrite(latchPin, LOW);      
      shiftOut(dataPin, clockPin, MSBFIRST, status);
      digitalWrite(latchPin, HIGH);
    }
    void setStatus(byte){
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, status);
      digitalWrite(latchPin, HIGH);
    }
    void initialize(){
      digitalWrite(latchPin, LOW);
      this->status = this->initValue;
      shiftOut(dataPin, clockPin, MSBFIRST, status);
      digitalWrite(latchPin, HIGH);
    }  
};

class digiSmart{
  private:
    bool status,prevStatus;
    bool initValue;
    int digiPin;
    //int latchPin,clockPin,dataPin;

  public:
    digiSmart(int pin,bool initVal=LOW){
      this->digiPin = pin;
      this->initValue = initVal;
      this->status = initVal;
      pinMode(this->digiPin,OUTPUT);
      digitalWrite(this->digiPin, initVal);      
      
    }
    void high(){
      prevStatus = status;
      status = HIGH;
      digitalWrite(digiPin, status);      
    }
    void low(){
      prevStatus = status;
      status = LOW;
      digitalWrite(digiPin, status);         
    }    
    void setPrevStatus(){ 
      if ( prevStatus != status){
        bool aux = status;
        status = prevStatus;
        prevStatus = aux;
      }      
      digitalWrite(digiPin, status); 
    }    
    void initialize(){
      status = initValue;
      prevStatus = initValue;      
      digitalWrite(digiPin, status);   
    }  
};
