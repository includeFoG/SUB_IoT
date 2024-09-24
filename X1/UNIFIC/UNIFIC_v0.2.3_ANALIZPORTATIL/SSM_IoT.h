
#ifndef _SSM_IoT_
//#include "Arduino.h"
//#include "Wire.h"
//#include "myShifft.h"

#ifdef SSM_003
#include "def003.h"
#endif
#ifdef SSM_005
#include "def005.h"
#endif

#define _SSM_IoT_

myShifft _shiffty(SHFT_STCP, SHFT_SHCP, SHFT_DATA);
digiSmart _5volts(EN_5VCC, LOW);  //EN 005 SE CONTROLA MEDIANTE SHIFTREG



class placa_IoT
{
  private:
    int _numplaca;
  public:
    placa_IoT(int numplaca)
    {
      _numplaca = numplaca;
    }



    void Initialize()
    {
      _shiffty.initialize();
    }

    void Initialize5V()
    {
      _5volts.initialize();
    }

    void RecStatus()
    {
      _shiffty.recStatus();
    }

    void SetPrevStatus()
    {
      _shiffty.setPrevStatus();
    }


    //ENABLES

    void EnableConmutado3v()
    {
#ifdef SSM_003
      _shiffty.set(EN_3VCC);
#endif

#ifdef SSM_005
      _shiffty.clear(EN_5VCC);
#endif
    }

    void EnableANA()
    {
#ifdef SSM_003
      _shiffty.set(EN_ANALOGS);
      _shiffty.set(EN_3VCC);
#endif

#ifdef SSM_005
      _shiffty.clear(EN_5VCC);
      _shiffty.set(EN_ANALOGS);
#endif
      delay(250);
    }

    void EnableMIKRO()
    {
#ifdef SSM_003
      _5volts.high();
      _shiffty.set(EN_3VCC);
      delay(1);
      _shiffty.set(EN_MIKRO);
#endif

#ifdef SSM_005
      _shiffty.clear(EN_5VCC);
      delay(1);
      _shiffty.set(EN_MIKRO);
#endif
    }

    void Enable5v()
    {
      _5volts.high();
    }


    void EnableRS485()
    {
      _shiffty.set(EN_RS485);
    }

    void EnableARDUCAM()
    {
      _shiffty.set(EN_ARDUCAM);
    }

    //DISABLES
    void DisableARDUCAM()
    {
      _shiffty.clear(EN_ARDUCAM);
    }

    void Disable5V()
    {
      _5volts.low();
    }


    void DisableRS485()
    {
      _shiffty.clear(EN_RS485);
    }


    void DisableConmutado3v()
    {
#ifdef SSM_003
      _shiffty.clear(EN_3VCC);
#endif

#ifdef SSM_005
      _shiffty.set(EN_5VCC);
#endif
    }


    void DisableANA()
    {
#ifdef SSM_003
      _shiffty.clear(EN_ANALOGS);
      _shiffty.clear(EN_3VCC);
#endif

#ifdef SSM_005
      _shiffty.clear(EN_ANALOGS);
      _shiffty.set(EN_5VCC);
#endif
      delay(250);
    }

    void DisableALL()
    {
#ifdef SSM_003
      _shiffty.initialize();
      _5volts.low();
#endif

#ifdef SSM_005
      _shiffty.initialize();
      // _5volts.low();
#endif
    }


    void DisableMIKRO()
    {
#ifdef SSM_003
      //modem.poweroff();
      //   delay(100);
      _shiffty.clear(EN_MIKRO);
#endif

#ifdef SSM_005
      //  modem.poweroff();
      //    delay(2000);
      //     _shiffty.initialize();
      //     _shiffty.set(EN_5VCC);
      _shiffty.clear(EN_MIKRO);
#endif
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //ver INIT_IO()
    // initcomport()

    void EnableD1()
    {
      pinMode(DIGI1, INPUT_PULLUP);
    }

    void EnableI2C()
    {
      pinMode(20, INPUT_PULLUP);
      pinMode(21, INPUT_PULLUP);
    }

    void EnableTEMP()
    {
      pinMode(TEMP, INPUT);
    }
};

#endif
