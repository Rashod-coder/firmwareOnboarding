#include "mbed.h"

AnalogIn APPS0(PA_3); 
AnalogIn APPS1(PA_4); 
AnalogIn brake_sensor(PA_0);
DigitalIn cockpit_switch(PA_1);
DigitalOut buzzer(PB_0); 
DigitalIn GLV_system(PA_5);
DigitalIn shutdown_circuit(PA_6);
Timer implausibility_timer;
Timer bse_timer;


int main() {
    // Constants for APPS sensor voltage ranges
    const float APPS0_MIN = 0.25f;
    const float APPS0_MAX = 2.25f;
    const float APPS1_MIN = 0.30f;
    const float APPS1_MAX = 2.7f;
    bool implausibility_detected = false;
    bool ready_to_drive = false;
    bool Tractive_System_Active = false;
    bool bse_detected = false;
    bool isBrakePressed = false;
    
    printf("Hello World\n");  

    
    while (true) {
        // Check if GLV system and shutdown circuit are active 
        if (shutdown_circuit.read()) {
        Tractive_System_Active = true;  
        } else {
        Tractive_System_Active = false;  
        }

        //Reads Acclerator Pedal Position Sensor Voltages
        float apps0_value = APPS0.read() * 3.3f; 
        float apps1_value = APPS1.read() * 3.3f; 

        //Reads Brake Sensor and Cockpit Switch States
        float brake_value = brake_sensor.read();
        float brake_voltage = brake_sensor.read() * 3.3f;
        float brake_percent = brake_value * 100.0f;
        int cockpit_switch_state = cockpit_switch.read();

        // Calculates percentage & averages it out
        float pos0 = (apps0_value - APPS0_MIN) / (APPS0_MAX - APPS0_MIN) * 100.0f;
        float pos1 = (apps1_value - APPS1_MIN) / (APPS1_MAX - APPS1_MIN) * 100.0f;
        float average_pos = (pos0 + pos1) / 2.0f;

        

        
        // Ready to Drive functionality
        if (brake_percent > 80.0f) {
            isBrakePressed = true;
            printf("Brake Pressed\n");
        } else {
            isBrakePressed = false;
        }
        if (!ready_to_drive){  
            if (isBrakePressed) {
                buzzer = 1;
                ThisThread::sleep_for(1s);
                buzzer = 0;
            }
            if (cockpit_switch_state == 1) {
                ready_to_drive = true;
                printf("1\n");
            }      
        
        }
        else {
                printf("APPS0 Position:  %f, APPS1 Position: %f\n", pos0, pos1);
                printf("Average Position: %f\n", average_pos);

                // If position goes out of ranges prints implausible. 
                if (pos0 <0.0f || pos0 > 100.0f) printf("Implausible\n");
                if (pos1 <0.0f || pos1 >100.0f) printf("Implausible\n");
                
                // Implausibility Detection
                float diff = fabs(pos0 - pos1);
                if (diff > 10.0f) {
                    if (!implausibility_detected) {
                        implausibility_timer.start();
                        implausibility_detected = true;
                    } else {
                        if (implausibility_timer.read() > 0.1f) {
                            printf("0\n");
                        }
                    }
                } 
                
                else {
                    implausibility_timer.stop();
                    implausibility_timer.reset();
                    implausibility_detected = false;
                }
            }

        // BSE Implasabuility
        if (brake_voltage < 0.0f || brake_voltage > 3.3f) {
            if (!bse_detected) {
                bse_timer.start();
                bse_detected = true;
            } else {
                if (bse_timer.read() > 0.1f) {
                    printf("2\n"); 
                }
            }
        } else {
            bse_timer.stop();
            bse_timer.reset();
            bse_detected = false;
        }
    
        ThisThread::sleep_for(100ms);
    
    }
}