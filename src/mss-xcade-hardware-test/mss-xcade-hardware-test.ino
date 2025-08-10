/*************************************************************************
Title:    MSS-XCADE Test Code
Authors:  Michael Petersen <railfan@drgw.net>
          Nathan D. Holmes <maverick@drgw.net>
          Iowa Scaled Engineering
File:     mss-xcade-test.ino
License:  GNU General Public License v3

LICENSE:
    Copyright (C) 2025 Michael Petersen & Nathan Holmes

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

*************************************************************************/
#include "Wire.h"
#include "mss-xcade.h"

WireMux wireMux;
XCade xcade;
XCade xcadeExpander1;


#define LOOP_UPDATE_TIME_MS 50
#define DEBUG_UPDATE_TIME_MS 250
void setup() 
{
  Serial.begin(115200);
  Serial.println("Startup");

  Wire.setPins(XCADE_I2C_SDA, XCADE_I2C_SCL);
  Wire.setClock(100000);
  Wire.begin();

  wireMux.begin(&Wire);

  xcade.begin(&wireMux);
}

uint8_t aspect = 0;
uint32_t testState = 0;
uint32_t mask = 0;
void loop() 
{
  uint32_t currentTime = millis();
  static uint32_t lastReadTime = 0;
  static uint32_t debugPrintfTime = 0;

	// Because debouncing needs some time between samples, don't go for a hideous update rate
  // 50mS or so between samples does nicely.  That gives a 200mS buffer for changes, which is more
  // than enough for propagation delay
	if (!(((uint32_t)currentTime - lastReadTime) > LOOP_UPDATE_TIME_MS))
    return;


  // Update the last time we ran through the loop to the current time
  lastReadTime = currentTime;


  // Just blink the RGB LED once a second in a nice dim of blue, so that we know the board is alive
  rgbLedWrite(XCADE_RGB_LED, 0, ((currentTime % 1000) > 500)?16:0, 0);

  // First, read the input state from the hardware
  xcade.updateInputs();
  //xcadeExpander1.updateInputs();


  if ((((uint32_t)currentTime - debugPrintfTime) > DEBUG_UPDATE_TIME_MS))
  {
    debugPrintfTime = currentTime;
    aspect += 2;
    if (aspect >= ASPECT_LUNAR)
      aspect = 1;
    xcade.signals.A1.setAspect((SignalAspect_t)aspect);
    xcade.signals.B1.setAspect((SignalAspect_t)aspect);
    xcade.signals.C1.setAspect((SignalAspect_t)aspect);
    xcade.signals.D1.setAspect((SignalAspect_t)aspect);
    xcade.signals.A2.setAspect((SignalAspect_t)aspect);
    xcade.signals.B2.setAspect((SignalAspect_t)aspect);
    xcade.signals.C2.setAspect((SignalAspect_t)aspect);
    xcade.signals.D2.setAspect((SignalAspect_t)aspect);


/*    Serial.printf("\n\nPort 1.A - ");
    xcade.mssPortA.printDebugStr();
    Serial.printf("\nPort 1.B - ");
    xcade.mssPortB.printDebugStr();
    Serial.printf("\nPort 1.C - ");
    xcade.mssPortC.printDebugStr();
    Serial.printf("\nPort 1.D - ");
    xcade.mssPortD.printDebugStr();*/


    switch(testState)
    {
      case 0:
        Serial.printf("Sensor Input test");
        testState = 1;
        mask = 0;

      case 1:
        Serial.printf("S1=[%c] S2=[%c] S3=[%c] S4=[%c] S5=[%c] S6=[%c] S7=[%c] S8=[%c] S9=[%c] S10=[%c]\n",
          xcade.gpio.digitalRead(SENSOR_1_PIN)?'*':' ',
          xcade.gpio.digitalRead(SENSOR_2_PIN)?'*':' ',
          xcade.gpio.digitalRead(SENSOR_3_PIN)?'*':' ',
          xcade.gpio.digitalRead(SENSOR_4_PIN)?'*':' ',
          xcade.gpio.digitalRead(SENSOR_5_PIN)?'*':' ',
          xcade.gpio.digitalRead(SENSOR_6_PIN)?'*':' ',
          xcade.gpio.digitalRead(SENSOR_7_PIN)?'*':' ',
          xcade.gpio.digitalRead(SENSOR_8_PIN)?'*':' ',
          xcade.gpio.digitalRead(SENSOR_9_PIN)?'*':' ',
          xcade.gpio.digitalRead(SENSOR_10_PIN)?'*':' ');

        if (xcade.gpio.digitalRead(SENSOR_10_PIN))
          testState = 10;
        break;

      case 10:
        Serial.printf("GPIO Input test\n");
        testState = 11;
        mask = 0;

      case 11:
        Serial.printf("G1=[%d] G2=[%d] G3=[%d] G4=[%d] G5=[%d] G6=[%d]\n",
          xcade.gpio.digitalRead(1),
          xcade.gpio.digitalRead(2),
          xcade.gpio.digitalRead(3),
          xcade.gpio.digitalRead(4),
          xcade.gpio.digitalRead(5),
          xcade.gpio.digitalRead(6));

        if (!xcade.gpio.digitalRead(6))
          testState = 15;

        break;

      case 15:
        Serial.printf("DIP Switch tests\n");
        testState = 16;
        mask = 0;

      case 16:
        Serial.printf("SW1=[%d] SW2=[%d] SW3=[%d] SW4=[%d] SW5=[%d] SW6=[%d] SW7=[%d]\n",
          xcade.configSwitches.getSwitch(1),
          xcade.configSwitches.getSwitch(2),
          xcade.configSwitches.getSwitch(3),
          xcade.configSwitches.getSwitch(4),
          xcade.configSwitches.getSwitch(5),
          xcade.configSwitches.getSwitch(6),
          xcade.configSwitches.getSwitch(7));

        if (xcade.configSwitches.getSwitch(7))
          testState = 20;

        break;




      case 20:
        Serial.printf("Port A/B Test\nPress Enter When Ready\n");
        if (!Serial.available())
          break;

        while(Serial.available())
          Serial.read();
        testState = 21;
        mask = 4;
        Serial.printf("Testing A->B Cleared\n");
        xcade.mssPortA.setLocalOccupancy(false);
        xcade.mssPortA.cascadeFromIndication(INDICATION_CLEAR, false);
        xcade.mssPortB.setLocalOccupancy(false);
        xcade.mssPortB.cascadeFromIndication(INDICATION_CLEAR, false);
        break;

      case 21:
        // Wait 4 cycles;
        if (mask-- > 0)
          break;
        mask = 4;
        if (xcade.mssPortA.indicationReceivedGet() != INDICATION_CLEAR)
        {
          Serial.printf("Port A not clear, fail\n");
          xcade.mssPortA.printDebugStr();
          Serial.printf("\n");
        } else if (xcade.mssPortB.indicationReceivedGet() != INDICATION_CLEAR)
        {
          Serial.printf("Port B not clear, fail\n");
          xcade.mssPortB.printDebugStr();
          Serial.printf("\n");
        } else {
          Serial.printf("Passed\n");
          testState++;
        }
        break;

      case 22:
        Serial.printf("Testing A->B A\n");
        xcade.mssPortA.cascadeFromIndication(INDICATION_STOP, false);
        testState = 23;
        mask = 4;
        break;
      
      case 23:
        // Wait 4 cycles;
        if (mask-- > 0)
          break;
        mask = 4;

        if (xcade.mssPortA.indicationReceivedGet() != INDICATION_CLEAR)
        {
          Serial.printf("Port A not clear, fail\n");
          xcade.mssPortA.printDebugStr();
          Serial.printf("\n");
        } else if (xcade.mssPortB.indicationReceivedGet() != INDICATION_APPROACH)
        {
          Serial.printf("Port B not receiving approach, fail\n");
          xcade.mssPortB.printDebugStr();
          Serial.printf("\n");
        } else {
          Serial.printf("Passed\n");
          testState++;
        }
        break;

      case 24:
        Serial.printf("Testing A->B AA\n");
        xcade.mssPortA.cascadeFromIndication(INDICATION_APPROACH, false);
        testState = 25;
        mask = 4;
        break;
      
      case 25:
        // Wait 4 cycles;
        if (mask-- > 0)
          break;
        mask = 4;

        if (xcade.mssPortA.indicationReceivedGet() != INDICATION_CLEAR)
        {
          Serial.printf("Port A not clear, fail\n");
          xcade.mssPortA.printDebugStr();
          Serial.printf("\n");
        } else if (xcade.mssPortB.indicationReceivedGet() != INDICATION_ADVANCE_APPROACH)
        {
          Serial.printf("Port B not receiving adv approach, fail\n");
          xcade.mssPortB.printDebugStr();
          Serial.printf("\n");
        } else {
          Serial.printf("Passed\n");
          testState++;
        }

        break;

      case 26:
        Serial.printf("Testing A->B DA\n");
        xcade.mssPortA.cascadeFromIndication(INDICATION_CLEAR, true);
        testState = 27;
        mask = 4;
        break;
      
      case 27:
        // Wait 4 cycles;
        if (mask-- > 0)
          break;
        mask = 4;

        if (xcade.mssPortA.indicationReceivedGet() != INDICATION_CLEAR)
        {
          Serial.printf("Port A not clear, fail\n");
          xcade.mssPortA.printDebugStr();
          Serial.printf("\n");
        } else if (xcade.mssPortB.indicationReceivedGet() != INDICATION_APPROACH_DIVERGING)
        {
          Serial.printf("Port B not receiving div approach, fail\n");
          xcade.mssPortB.printDebugStr();
          Serial.printf("\n");
        } else {
          Serial.printf("Passed\n");
          testState++;
        }
        break;

      case 28:
        Serial.printf("Testing A->B S\n");
        xcade.mssPortA.setLocalOccupancy(true);
        xcade.mssPortA.cascadeFromIndication(INDICATION_CLEAR, false);
        testState++;
        mask = 4;
        break;
      
      case 29:
        // Wait 4 cycles;
        if (mask-- > 0)
          break;
        mask = 4;

        if (xcade.mssPortA.indicationReceivedGet() != INDICATION_STOP)
        {
          Serial.printf("Port A not receiving stop, fail\n");
          xcade.mssPortA.printDebugStr();
          Serial.printf("\n");
        } else if (xcade.mssPortB.indicationReceivedGet() != INDICATION_STOP)
        {
          Serial.printf("Port B not receiving stop, fail\n");
          xcade.mssPortB.printDebugStr();
          Serial.printf("\n");
        } else {
          Serial.printf("Passed\n");
          testState++;
        }
        break;

      case 30:
        testState++;
        mask = 4;
        Serial.printf("Testing B->A Cleared\n");
        xcade.mssPortA.setLocalOccupancy(false);
        xcade.mssPortA.cascadeFromIndication(INDICATION_CLEAR, false);
        xcade.mssPortB.setLocalOccupancy(false);
        xcade.mssPortB.cascadeFromIndication(INDICATION_CLEAR, false);
        break;

      case 31:
        // Wait 4 cycles;
        if (mask-- > 0)
          break;
        mask = 4;

        if (xcade.mssPortA.indicationReceivedGet() != INDICATION_CLEAR)
        {
          Serial.printf("Port A not clear, fail\n");
          xcade.mssPortA.printDebugStr();
          Serial.printf("\n");
        } else if (xcade.mssPortB.indicationReceivedGet() != INDICATION_CLEAR)
        {
          Serial.printf("Port B not clear, fail\n");
          xcade.mssPortB.printDebugStr();
          Serial.printf("\n");
        } else {
          Serial.printf("Passed\n");
          testState++;
        }
        break;

      case 32:
        Serial.printf("Testing B->A A\n");
        xcade.mssPortB.cascadeFromIndication(INDICATION_STOP, false);
        testState++;
        mask = 4;
        break;
      
      case 33:
        // Wait 4 cycles;
        if (mask-- > 0)
          break;

        mask = 4;

        if (xcade.mssPortB.indicationReceivedGet() != INDICATION_CLEAR)
        {
          Serial.printf("Port B not clear, fail\n");
          xcade.mssPortB.printDebugStr();
          Serial.printf("\n");
        } else if (xcade.mssPortA.indicationReceivedGet() != INDICATION_APPROACH)
        {
          Serial.printf("Port A not receiving approach, fail\n");
          xcade.mssPortA.printDebugStr();
          Serial.printf("\n");
        } else {
          Serial.printf("Passed\n");
          testState++;
        }
        break;

      case 34:
        Serial.printf("Testing B->A AA\n");
        xcade.mssPortB.cascadeFromIndication(INDICATION_APPROACH, false);
        testState++;
        mask = 4;
        break;
      
      case 35:
        // Wait 4 cycles;
        if (mask-- > 0)
          break;

        mask = 4;

        if (xcade.mssPortB.indicationReceivedGet() != INDICATION_CLEAR)
        {
          Serial.printf("Port B not clear, fail\n");
          xcade.mssPortB.printDebugStr();
          Serial.printf("\n");
        } else if (xcade.mssPortA.indicationReceivedGet() != INDICATION_ADVANCE_APPROACH)
        {
          Serial.printf("Port A not receiving adv approach, fail\n");
          xcade.mssPortA.printDebugStr();
          Serial.printf("\n");
        } else {
          Serial.printf("Passed\n");
          testState++;
        }
        break;

      case 36:
        Serial.printf("Testing B->A DA\n");
        xcade.mssPortB.cascadeFromIndication(INDICATION_CLEAR, true);
        testState++;
        mask = 4;
        break;
      
      case 37:
        // Wait 4 cycles;
        if (mask-- > 0)
          break;

        mask = 4;

        if (xcade.mssPortB.indicationReceivedGet() != INDICATION_CLEAR)
        {
          Serial.printf("Port B not clear, fail\n");
          xcade.mssPortB.printDebugStr();
          Serial.printf("\n");
        } else if (xcade.mssPortA.indicationReceivedGet() != INDICATION_APPROACH_DIVERGING)
        {
          Serial.printf("Port A not receiving div approach, fail\n");
          xcade.mssPortA.printDebugStr();
          Serial.printf("\n");
        } else {
          Serial.printf("Passed\n");
          testState++;
        }
        break;

      case 38:
        Serial.printf("Testing B->A S\n");
        xcade.mssPortB.setLocalOccupancy(true);
        xcade.mssPortB.cascadeFromIndication(INDICATION_CLEAR, false);
        testState++;
        mask = 4;
        break;
      
      case 39:
        // Wait 4 cycles;
        if (mask-- > 0)
          break;

        mask = 4;

        if (xcade.mssPortA.indicationReceivedGet() != INDICATION_STOP)
        {
          Serial.printf("Port A not receiving stop, fail\n");
          xcade.mssPortA.printDebugStr();
          Serial.printf("\n");
        } else if (xcade.mssPortB.indicationReceivedGet() != INDICATION_STOP)
        {
          Serial.printf("Port B not receiving stop, fail\n");
          xcade.mssPortB.printDebugStr();
          Serial.printf("\n");
        } else {
          Serial.printf("Passed\n");
          testState++;
        }
        break;

      // Switch to C/D ports
      case 40:
        Serial.printf("Port C/D Test\nPress Enter When Ready\n");
        if (!Serial.available())
          break;

        while(Serial.available())
          Serial.read();
        testState++;
        mask = 4;
        Serial.printf("Testing C->D Cleared\n");

        xcade.mssPortC.setLocalOccupancy(false);
        xcade.mssPortC.cascadeFromIndication(INDICATION_CLEAR, false);
        xcade.mssPortD.setLocalOccupancy(false);
        xcade.mssPortD.cascadeFromIndication(INDICATION_CLEAR, false);
        break;

      case 41:
        // Wait 4 cycles;
        if (mask-- > 0)
          break;

        mask = 4;

        if (xcade.mssPortC.indicationReceivedGet() != INDICATION_CLEAR)
        {
          Serial.printf("Port C not clear, fail\n");
          xcade.mssPortC.printDebugStr();
          Serial.printf("\n");
        } else if (xcade.mssPortD.indicationReceivedGet() != INDICATION_CLEAR)
        {
          Serial.printf("Port D not clear, fail\n");
          xcade.mssPortD.printDebugStr();
          Serial.printf("\n");
        } else {
          Serial.printf("Passed\n");
          testState++;
        }
        break;

      case 42:
        Serial.printf("Testing C->D A\n");
        xcade.mssPortC.cascadeFromIndication(INDICATION_STOP, false);
        testState++;
        mask = 4;
        break;
      
      case 43:
        // Wait 4 cycles;
        if (mask-- > 0)
          break;

        mask = 4;

        if (xcade.mssPortC.indicationReceivedGet() != INDICATION_CLEAR)
        {
          Serial.printf("Port C not clear, fail\n");
          xcade.mssPortC.printDebugStr();
          Serial.printf("\n");
        } else if (xcade.mssPortD.indicationReceivedGet() != INDICATION_APPROACH)
        {
          Serial.printf("Port D not receiving approach, fail\n");
          xcade.mssPortD.printDebugStr();
          Serial.printf("\n");
        } else {
          Serial.printf("Passed\n");
          testState++;
        }
        break;

      case 44:
        Serial.printf("Testing C->D AA\n");
        xcade.mssPortC.cascadeFromIndication(INDICATION_APPROACH, false);
        testState++;
        mask = 4;
        break;
      
      case 45:
        // Wait 4 cycles;
        if (mask-- > 0)
          break;

        mask = 4;

        if (xcade.mssPortC.indicationReceivedGet() != INDICATION_CLEAR)
        {
          Serial.printf("Port C not clear, fail\n");
          xcade.mssPortC.printDebugStr();
          Serial.printf("\n");
        } else if (xcade.mssPortD.indicationReceivedGet() != INDICATION_ADVANCE_APPROACH)
        {
          Serial.printf("Port D not receiving adv approach, fail\n");
          xcade.mssPortD.printDebugStr();
          Serial.printf("\n");
        } else {
          Serial.printf("Passed\n");
          testState++;
        }
        break;

      case 46:
        Serial.printf("Testing C->D DA\n");
        xcade.mssPortC.cascadeFromIndication(INDICATION_CLEAR, true);
        testState++;
        mask = 4;
        break;
      
      case 47:
        // Wait 4 cycles;
        if (mask-- > 0)
          break;

        mask = 4;

        if (xcade.mssPortC.indicationReceivedGet() != INDICATION_CLEAR)
        {
          Serial.printf("Port C not clear, fail\n");
          xcade.mssPortC.printDebugStr();
          Serial.printf("\n");
        } else if (xcade.mssPortD.indicationReceivedGet() != INDICATION_APPROACH_DIVERGING)
        {
          Serial.printf("Port D not receiving div approach, fail\n");
          xcade.mssPortD.printDebugStr();
          Serial.printf("\n");
        } else {
          Serial.printf("Passed\n");
          testState++;
        }
        break;

      case 48:
        Serial.printf("Testing C->D S\n");
        xcade.mssPortC.setLocalOccupancy(true);
        xcade.mssPortC.cascadeFromIndication(INDICATION_CLEAR, false);
        testState++;
        mask = 4;
        break;
      
      case 49:
        // Wait 4 cycles;
        if (mask-- > 0)
          break;

        mask = 4;

        if (xcade.mssPortC.indicationReceivedGet() != INDICATION_STOP)
        {
          Serial.printf("Port C not receiving stop, fail\n");
          xcade.mssPortC.printDebugStr();
          Serial.printf("\n");
        } else if (xcade.mssPortD.indicationReceivedGet() != INDICATION_STOP)
        {
          Serial.printf("Port D not receiving stop, fail\n");
          xcade.mssPortD.printDebugStr();
          Serial.printf("\n");
        } else {
          Serial.printf("Passed\n");
          testState++;
        }
        break;

      case 50:
        testState++;
        mask = 4;
        Serial.printf("Testing D->C Cleared\n");

        xcade.mssPortC.setLocalOccupancy(false);
        xcade.mssPortC.cascadeFromIndication(INDICATION_CLEAR, false);
        xcade.mssPortD.setLocalOccupancy(false);
        xcade.mssPortD.cascadeFromIndication(INDICATION_CLEAR, false);
        break;

      case 51:
        // Wait 4 cycles;
        if (mask-- > 0)
          break;
        
        mask = 4;

        if (xcade.mssPortC.indicationReceivedGet() != INDICATION_CLEAR)
        {
          Serial.printf("Port C not clear, fail\n");
          xcade.mssPortC.printDebugStr();
          Serial.printf("\n");
        } else if (xcade.mssPortD.indicationReceivedGet() != INDICATION_CLEAR)
        {
          Serial.printf("Port D not clear, fail\n");
          xcade.mssPortD.printDebugStr();
          Serial.printf("\n");
        } else {
          Serial.printf("Passed\n");
          testState++;
        }
        break;

      case 52:
        Serial.printf("Testing D->C A\n");
        xcade.mssPortD.cascadeFromIndication(INDICATION_STOP, false);
        testState++;
        mask = 4;
        break;
      
      case 53:
        // Wait 4 cycles;
        if (mask-- > 0)
          break;

        mask = 4;

        if (xcade.mssPortD.indicationReceivedGet() != INDICATION_CLEAR)
        {
          Serial.printf("Port D not clear, fail\n");
          xcade.mssPortD.printDebugStr();
          Serial.printf("\n");
        } else if (xcade.mssPortC.indicationReceivedGet() != INDICATION_APPROACH)
        {
          Serial.printf("Port C not receiving approach, fail\n");
          xcade.mssPortC.printDebugStr();
          Serial.printf("\n");
        } else {
          Serial.printf("Passed\n");
          testState++;
        }
        break;

      case 54:
        Serial.printf("Testing D->C AA\n");
        xcade.mssPortD.cascadeFromIndication(INDICATION_APPROACH, false);
        testState++;
        mask = 4;
        break;
      
      case 55:
        // Wait 4 cycles;
        if (mask-- > 0)
          break;

        mask = 4;

        if (xcade.mssPortD.indicationReceivedGet() != INDICATION_CLEAR)
        {
          Serial.printf("Port D not clear, fail\n");
          xcade.mssPortD.printDebugStr();
          Serial.printf("\n");
        } else if (xcade.mssPortC.indicationReceivedGet() != INDICATION_ADVANCE_APPROACH)
        {
          Serial.printf("Port C not receiving adv approach, fail\n");
          xcade.mssPortC.printDebugStr();
          Serial.printf("\n");
        } else {
          Serial.printf("Passed\n");
          testState++;
        }
        break;

      case 56:
        Serial.printf("Testing D->C DA\n");
        xcade.mssPortD.cascadeFromIndication(INDICATION_CLEAR, true);
        testState++;
        mask = 4;
        break;
      
      case 57:
        // Wait 4 cycles;
        if (mask-- > 0)
          break;

        mask = 4;

        if (xcade.mssPortD.indicationReceivedGet() != INDICATION_CLEAR)
        {
          Serial.printf("Port D not clear, fail\n");
          xcade.mssPortD.printDebugStr();
          Serial.printf("\n");
        } else if (xcade.mssPortC.indicationReceivedGet() != INDICATION_APPROACH_DIVERGING)
        {
          Serial.printf("Port C not receiving div approach, fail\n");
          xcade.mssPortC.printDebugStr();
          Serial.printf("\n");
        } else {
          Serial.printf("Passed\n");
          testState++;
        }
        break;

      case 58:
        Serial.printf("Testing D->C S\n");
        xcade.mssPortD.setLocalOccupancy(true);
        xcade.mssPortD.cascadeFromIndication(INDICATION_CLEAR, false);
        testState++;
        mask = 4;
        break;
      
      case 59:
        // Wait 4 cycles;
        if (mask-- > 0)
          break;

        mask = 4;

        if (xcade.mssPortC.indicationReceivedGet() != INDICATION_STOP)
        {
          Serial.printf("Port C not receiving stop, fail\n");
          xcade.mssPortC.printDebugStr();
          Serial.printf("\n");
        } else if (xcade.mssPortD.indicationReceivedGet() != INDICATION_STOP)
        {
          Serial.printf("Port D not receiving stop, fail\n");
          xcade.mssPortD.printDebugStr();
          Serial.printf("\n");
        } else {
          Serial.printf("Passed\n");
          testState++;
        }
        break;

      case 60:
        Serial.printf("Testing passed\n");
        break;


    }

  }


  // Now that all state is computed, send the outputs to the hardware
  xcade.updateOutputs();
//  xcadeExpander1.updateOutputs();
}
