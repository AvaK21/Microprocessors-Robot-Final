 /*
 * MAIN Generated Driver File
 * 
 * @file main.c
 * 
 * @defgroup main MAIN
 * 
 * @brief This is the generated driver implementation file for the MAIN driver.
 *
 * @version MAIN Driver Version 1.0.2
 *
 * @version Package Version: 3.1.2
*/

/*
© [2026] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/
#include "mcc_generated_files/system/system.h"

#include <stdint.h>

/*
    Main application
*/
volatile bool update = false;
bool last_instruction_stop = false;
    uint32_t Radc;
    uint32_t Fadc;
    uint32_t Ladc;
    uint32_t Lspeed;
    uint32_t Rspeed;
    
    const uint16_t CLEAR_FRONT = 1050;
    const uint16_t RIGHT_CLOSE = 1400;
    //works with 1000 right far 
    const uint16_t RIGHT_FAR = 1200;
    const uint16_t LEFT_CLOSE = 1400;
    const uint16_t RIGHT_STRAIGHT = 420;
    const uint16_t LEFT_STRAIGHT = 460;
    const uint16_t RIGHT_ADJUST_RW = 330;
    const uint16_t RIGHT_ADJUST_LW = 430;
    const uint16_t LEFT_ADJUST_RW = 380;
    const uint16_t LEFT_ADJUST_LW = 360;
    const uint16_t RIGHT_BACKWARDS = 300;
    const uint16_t LEFT_BACKWARDS = 300;
    const uint16_t FRONT_WALL_CLOSE = 2700;
    
    
    const uint32_t LOW_THRESHOLD = 1200;
    const uint32_t HIGH_THRESHOLD = 1450;
    const uint16_t ADJUST = 15;
    
uint32_t Get_Smoothed_ADC(adc_channel_t channel) {
    uint32_t acc = 0;
    for(int i = 0; i < 10; i++) {
        acc += ADC_ChannelSelectAndConvert(channel);
    }
    acc = acc / 10;
    return acc; // Divide by 16
    
}
void JumpStart(){
    PWM1_16BIT_SetSlice1Output1DutyCycleRegister(LEFT_STRAIGHT);
    PWM1_16BIT_SetSlice1Output2DutyCycleRegister(RIGHT_STRAIGHT);
    PWM1_16BIT_LoadBufferRegisters();
    __delay_ms(50);
    last_instruction_stop = false;
}

void Backup(){
    PWM1_16BIT_SetSlice1Output1DutyCycleRegister(0);
    PWM1_16BIT_SetSlice1Output2DutyCycleRegister(0);
    PWM1_16BIT_LoadBufferRegisters();
    __delay_ms(200);
    Fadc = Get_Smoothed_ADC(IR_front);
    while (Fadc > FRONT_WALL_CLOSE){

        L_DIR_SetLow();
        R_DIR_SetLow();
        JumpStart();
        PWM1_16BIT_SetSlice1Output1DutyCycleRegister(LEFT_BACKWARDS);
        PWM1_16BIT_SetSlice1Output2DutyCycleRegister(RIGHT_BACKWARDS);
        PWM1_16BIT_LoadBufferRegisters();
        __delay_ms(5);
        PWM1_16BIT_SetSlice1Output1DutyCycleRegister(0);
        PWM1_16BIT_SetSlice1Output2DutyCycleRegister(0);
        PWM1_16BIT_LoadBufferRegisters();
    __delay_ms(200);
        Fadc = Get_Smoothed_ADC(IR_front);
    }
    __delay_ms(200);
    L_DIR_SetHigh();
    R_DIR_SetHigh();

}
void Drive_Unitl_Right_Wall_Found(){
    L_DIR_SetHigh();
    R_DIR_SetHigh();
    PWM1_16BIT_SetSlice1Output1DutyCycleRegister(LEFT_STRAIGHT);
    PWM1_16BIT_SetSlice1Output2DutyCycleRegister(RIGHT_STRAIGHT);
    PWM1_16BIT_LoadBufferRegisters();

    uint32_t temp_Radc = 0;
    uint32_t temp_Fadc = 0;
    while(temp_Radc < 800 && temp_Fadc < FRONT_WALL_CLOSE ) { // 800 is the "Wall is back" threshold
        temp_Radc = Get_Smoothed_ADC(IR_right);
        temp_Fadc = Get_Smoothed_ADC(IR_front);
        __delay_ms(5); // Small poll rate
    }
    if (temp_Fadc >= FRONT_WALL_CLOSE){
        Backup();
        // have function for spinning 90 degrees to the right
    }else {
        __delay_ms(5);
        PWM1_16BIT_SetSlice1Output1DutyCycleRegister(0);
        PWM1_16BIT_SetSlice1Output2DutyCycleRegister(0);
        PWM1_16BIT_LoadBufferRegisters();
        __delay_ms(100);
        last_instruction_stop = true;
    }
}

void Adjust_Too_Far_Angle(){



    // Stop
    PWM1_16BIT_SetSlice1Output1DutyCycleRegister(0);
    PWM1_16BIT_SetSlice1Output2DutyCycleRegister(0);
    PWM1_16BIT_LoadBufferRegisters();
    __delay_ms(200);

    // Nudge left to straighten parallel to wall
    L_DIR_SetHigh();
    R_DIR_SetLow();
    PWM1_16BIT_SetSlice1Output1DutyCycleRegister(250);
    PWM1_16BIT_SetSlice1Output2DutyCycleRegister(250);
    PWM1_16BIT_LoadBufferRegisters();

    while (Radc < LOW_THRESHOLD + 10) {
        Radc = Get_Smoothed_ADC(IR_right);
        __delay_ms(5);
    }


    L_DIR_SetHigh();
    R_DIR_SetHigh();

    
}

void Adjust_Too_Close_Angle(){

    // Back straight up
    L_DIR_SetLow();
    R_DIR_SetLow();
    //JumpStart();
    PWM1_16BIT_SetSlice1Output1DutyCycleRegister(LEFT_BACKWARDS);
    PWM1_16BIT_SetSlice1Output2DutyCycleRegister(RIGHT_BACKWARDS);
    PWM1_16BIT_LoadBufferRegisters();
    __delay_ms(200);


    // Stop
    PWM1_16BIT_SetSlice1Output1DutyCycleRegister(0);
    PWM1_16BIT_SetSlice1Output2DutyCycleRegister(0);
    PWM1_16BIT_LoadBufferRegisters();
    __delay_ms(250);

    // Nudge left to straighten parallel to wall
    L_DIR_SetLow();
    R_DIR_SetHigh();
    PWM1_16BIT_SetSlice1Output1DutyCycleRegister(250);
    PWM1_16BIT_SetSlice1Output2DutyCycleRegister(250);
    PWM1_16BIT_LoadBufferRegisters();

    while (Radc > HIGH_THRESHOLD - 15) {
        Radc = Get_Smoothed_ADC(IR_right);
        __delay_ms(5);
    }

    L_DIR_SetHigh();
    R_DIR_SetHigh();

    
}

void Right_Forward_Turn(){
    //Hole detected
    PWM1_16BIT_SetSlice1Output1DutyCycleRegister(0);
    PWM1_16BIT_SetSlice1Output2DutyCycleRegister(0);
    PWM1_16BIT_LoadBufferRegisters();
    __delay_ms(200);
    //Go forward to pass the wall (go half of a wall)
    PWM1_16BIT_SetSlice1Output1DutyCycleRegister(LEFT_STRAIGHT);
    PWM1_16BIT_SetSlice1Output2DutyCycleRegister(RIGHT_STRAIGHT);
    PWM1_16BIT_LoadBufferRegisters();
    __delay_ms(570);
    //Pause
    PWM1_16BIT_SetSlice1Output1DutyCycleRegister(0);
    PWM1_16BIT_SetSlice1Output2DutyCycleRegister(0);
    PWM1_16BIT_LoadBufferRegisters();
    Fadc = Get_Smoothed_ADC(IR_front);
    if (Fadc > CLEAR_FRONT){
           
        Backup();
    }
    __delay_ms(100);
    //Turn 90 degrees
    L_DIR_SetHigh();
    R_DIR_SetLow();
    PWM1_16BIT_SetSlice1Output1DutyCycleRegister(LEFT_STRAIGHT);
    PWM1_16BIT_SetSlice1Output2DutyCycleRegister(RIGHT_STRAIGHT);
    PWM1_16BIT_LoadBufferRegisters();
    __delay_ms(550);
    //Pause
    PWM1_16BIT_SetSlice1Output1DutyCycleRegister(0);
    PWM1_16BIT_SetSlice1Output2DutyCycleRegister(0);       
    PWM1_16BIT_LoadBufferRegisters();
    __delay_ms(500);
    
    //Arbutary go forward to sense next wall or hole
    //Drive_Unitl_Right_Wall_Found();
    R_DIR_SetHigh();
    PWM1_16BIT_SetSlice1Output1DutyCycleRegister(LEFT_STRAIGHT);
    PWM1_16BIT_SetSlice1Output2DutyCycleRegister(RIGHT_STRAIGHT);
    PWM1_16BIT_LoadBufferRegisters();
    //works with 300 no lower 
    __delay_ms(550);
    PWM1_16BIT_SetSlice1Output1DutyCycleRegister(0);
    PWM1_16BIT_SetSlice1Output2DutyCycleRegister(0);
    PWM1_16BIT_LoadBufferRegisters();
    __delay_ms(250);
    Radc = Get_Smoothed_ADC(IR_right);
    //last_instruction_stop = true;
    if (Radc > HIGH_THRESHOLD - 100){
    Adjust_Too_Close_Angle();
    }

}





void Timer0_Interrupt(void){
    //printf("Timer0 interrupt test\n");

    update = true;


    TMR0_CounterSet(0);
    
}

int main(void)
{
    SYSTEM_Initialize();
    TMR0_Initialize();
    PWM1_16BIT_Initialize();
    PWM1_16BIT_Enable();

    INTERRUPT_GlobalInterruptEnable(); 
    Timer0_OverflowCallbackRegister(Timer0_Interrupt);

    const uint16_t PWM_PERIOD = 1000; // Based on your 1kHz frequency @ 1MHz clock

    Timer0_Start();
      
    L_DIR_SetHigh();
    R_DIR_SetHigh();


    while(1)
    {
        if (update){
            Radc = Get_Smoothed_ADC(IR_right);
            Fadc = Get_Smoothed_ADC(IR_front);
            Ladc = Get_Smoothed_ADC(IR_left);

            if (Radc < 500){
            L_DIR_SetHigh();
            R_DIR_SetHigh();
                Right_Forward_Turn();

            }

            else if (Fadc < CLEAR_FRONT){ 
                L_DIR_SetHigh();
                R_DIR_SetHigh();
                if (last_instruction_stop){
                    PWM1_16BIT_SetSlice1Output1DutyCycleRegister(LEFT_STRAIGHT);
                    PWM1_16BIT_SetSlice1Output2DutyCycleRegister(RIGHT_STRAIGHT);
                    PWM1_16BIT_LoadBufferRegisters();
                    __delay_ms(50);
                    last_instruction_stop = false;
                }
                if (Radc > HIGH_THRESHOLD){    // Too close to right wall -> Veer Left
                    Adjust_Too_Close_Angle();

                } 

                else if (Radc < LOW_THRESHOLD ){  // Moving away from right wall -> Veer Right
                    Lspeed = LEFT_STRAIGHT + ADJUST;
                    Rspeed = RIGHT_STRAIGHT - ADJUST;
                } else {

                    Lspeed = LEFT_STRAIGHT;
                    Rspeed = RIGHT_STRAIGHT;  

                }

                PWM1_16BIT_SetSlice1Output1DutyCycleRegister(Lspeed);
                PWM1_16BIT_SetSlice1Output2DutyCycleRegister(Rspeed);

                PWM1_16BIT_LoadBufferRegisters();
                
                       

//To close to the front wall
            } else if ( Fadc >= CLEAR_FRONT){

                if (Fadc > FRONT_WALL_CLOSE) {
                    Backup();

                } else {
                    if (Ladc <700) { // turn left 90

                    L_DIR_SetLow();
                    R_DIR_SetHigh();

                    PWM1_16BIT_SetSlice1Output1DutyCycleRegister(LEFT_STRAIGHT);
                    PWM1_16BIT_SetSlice1Output2DutyCycleRegister(RIGHT_STRAIGHT);
                    PWM1_16BIT_LoadBufferRegisters();
                    __delay_ms(570);
                    //Pause
                    PWM1_16BIT_SetSlice1Output1DutyCycleRegister(0);
                    PWM1_16BIT_SetSlice1Output2DutyCycleRegister(0);       
                    PWM1_16BIT_LoadBufferRegisters();
                    __delay_ms(500);
                    } else {// turn 180 (on purpose a little less than 180)
                    Adjust_Too_Close_Angle();

                    L_DIR_SetLow();
                    R_DIR_SetHigh();

                    PWM1_16BIT_SetSlice1Output1DutyCycleRegister(LEFT_STRAIGHT);
                    PWM1_16BIT_SetSlice1Output2DutyCycleRegister(RIGHT_STRAIGHT);
                    PWM1_16BIT_LoadBufferRegisters();
                    __delay_ms(1180);
                    //Pause
                    PWM1_16BIT_SetSlice1Output1DutyCycleRegister(0);
                    PWM1_16BIT_SetSlice1Output2DutyCycleRegister(0);       
                    PWM1_16BIT_LoadBufferRegisters();
                    __delay_ms(500);
                    }
                }
            }
            else {
                //Right now just stop
                PWM1_16BIT_SetSlice1Output1DutyCycleRegister(0);
                PWM1_16BIT_SetSlice1Output2DutyCycleRegister(0);
                last_instruction_stop = true;
            }
           
            PWM1_16BIT_LoadBufferRegisters();
            update = false;
        }
    }    
}