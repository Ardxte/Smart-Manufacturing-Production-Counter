// EFB2073:Microprocessor & Computer Architecture - May 2026
// Group Project: Smart Manufacturing Production Counter
// Group Name: OWL
// Group Members: Ardiansyah 		        		| 24003826
// 				         Wildan Owais Al Bukhari 	| 24004522


#include "mbed.h"
#include "TextLCD.h" // Required for TextLCD library
#include <stdio.h>   // Required for sprintf

// ==========================================
// 1. HARDWARE PIN DEFINITIONS
// ==========================================

DigitalIn btnStart(PC_0, PullUp);  // Start button - starts production or adds one accepted product
DigitalIn btnStop(PC_1, PullUp);   // Stop button - pauses the production process
DigitalIn btnReject(PC_2, PullUp); // Reject button - adds one rejected product
DigitalIn btnReset(PC_3, PullUp);  // Reset button - clears the current production data

DigitalOut ledRunning(PA_0); // Shows that production is currently running
DigitalOut ledTarget(PA_1);  // Turns on when the production target is reached
DigitalOut ledFault(PA_4);   // Turns on when the reject rate is above 10%

PortOut segments(PortB, 0xFF); // Controls the segments of the 7-segment displays
DigitalOut dig1(PB_12);        // First 7-segment digit
DigitalOut dig2(PB_13);        // Second 7-segment digit
DigitalOut dig3(PB_14);        // Third 7-segment digit
DigitalOut dig4(PB_15);        // Fourth 7-segment digit

// TextLCD initialization (RS, E, D4, D5, D6, D7)
TextLCD lcd(PC_4, PC_5, PC_6, PC_7, PC_8, PC_9); // LCD is used to show production information and system status


// ==========================================
// 2. 7-SEGMENT MULTIPLEXING LOGIC
// ==========================================

const int DIGIT_ON  = 1; 
const int DIGIT_OFF = 0; 

// Segment patterns for numbers 0-9. Each value tells the display which segments to turn on.
const uint8_t digit_map[10] = { 0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90 };

// Stores the four digits that will be shown on the 7-segment displays
volatile uint8_t display_digits[4] = {0, 0, 0, 0};

Ticker display_ticker; // Used to refresh the 7-segment display continuously

void refreshDisplay() {
    static int current_digit = 0; // Keeps track of which digit is being updated

    dig1 = DIGIT_OFF; dig2 = DIGIT_OFF; dig3 = DIGIT_OFF; dig4 = DIGIT_OFF; // Turn all digits off before changing the display

    segments = digit_map[display_digits[current_digit]]; // Set the correct segments for the current number

    // Only one digit is turned on at a time to multiplex the four displays
    switch(current_digit) {
        case 0: dig1 = DIGIT_ON; break; // Turn on first digit
        case 1: dig2 = DIGIT_ON; break; // Turn on second digit
        case 2: dig3 = DIGIT_ON; break; // Turn on third digit
        case 3: dig4 = DIGIT_ON; break; // Turn on fourth digit
    }

    current_digit++; // Move to the next digit
    if (current_digit > 3) current_digit = 0; // Start again from the first digit after the fourth
}


// ==========================================
// 3. MAIN SYSTEM LOGIC & STATE MACHINE
// ==========================================

// These states show the current condition of the production system
enum SystemState { READY, RUNNING, STOPPED, COMPLETE };

int main() {
    lcd.cls(); // Clear the LCD when the system starts

    display_ticker.attach(&refreshDisplay, 0.005f); // Refresh the 7-segment display every 5 ms
    
    SystemState state = READY; // Start in READY state and wait for the Start button
    
    int accepted_count = 0; // Keeps track of successfully produced products
    int rejected_count = 0; // Keeps track of rejected products
    const int TARGET_COUNT = 20; // Set the production target to 20 products
    
    // Counter used to control the timing of the LCD warning
    int blink_counter = 0; 
    
    // Character arrays used to store the two lines shown on the LCD
    char lcd_line1[17];
    char lcd_line2[17];

    while (true) {
        // --- 1. MATH & ANALYTICS ---
        
        int total_produced = accepted_count + rejected_count; // Total products includes both accepted and rejected products
        int efficiency = 100; // Default efficiency is 100% before any products are made
        bool is_fault = false; // Used to keep track of whether the reject rate is too high
        
        if (total_produced > 0) {
            efficiency = (accepted_count * 100) / total_produced; // Calculate efficiency based on accepted products out of total products

            // Fault is triggered when more than 10% of the products are rejected
            if (((rejected_count * 100) / total_produced) > 10) {
                is_fault = true;
            }
        }
        
        // --- 2. BUTTON INPUT LOGIC (With Debouncing) ---

        if (btnStart == 0) { // Check if the Start button is pressed

            if (state == READY || state == STOPPED) {
                state = RUNNING; // Start or resume production
            } else if (state == RUNNING) {
                accepted_count++; // Each press while running represents one successfully produced product

                if (accepted_count >= TARGET_COUNT) {
                    state = COMPLETE; // Stop counting once the production target has been reached
                }
            }

            ThisThread::sleep_for(250); // Short delay to prevent one button press being detected multiple times
        }
        
        if (btnStop == 0) { // Check if the Stop button is pressed
            if (state == RUNNING) state = STOPPED; // Pause production while keeping all current data
            ThisThread::sleep_for(250); // Button debounce delay
        }
        
        if (btnReject == 0) { // Check if the Reject button is pressed
            if (state == RUNNING) rejected_count++; // Add one rejected product without increasing accepted count
            ThisThread::sleep_for(250); // Button debounce delay
        }
        
        if (btnReset == 0) { // Check if the Reset button is pressed
            state = READY; // Return the system to the starting state
            accepted_count = 0; // Clear accepted product count
            rejected_count = 0; // Clear rejected product count
            blink_counter = 0; // Reset the LCD warning timer
            ThisThread::sleep_for(250); // Button debounce delay
        }

        // --- 3. HARDWARE UPDATES (LEDs & 7-Segment) ---

        // Split the accepted product count into separate digits for the 7-segment displays
        display_digits[0] = (accepted_count / 1000) % 10;
        display_digits[1] = (accepted_count / 100) % 10;
        display_digits[2] = (accepted_count / 10) % 10;
        display_digits[3] = accepted_count % 10;
        
        ledRunning = (state == RUNNING) ? 1 : 0; // Running LED is on only while production is active
        ledTarget  = (state == COMPLETE) ? 1 : 0; // Target LED turns on when the target is reached
        ledFault   = (is_fault) ? 1 : 0; // Fault LED turns on when reject rate is above 10%

        // --- 4. BLINK TIMER LOGIC ---

        blink_counter++; // Increase the counter each time the main loop runs

        // 40 loops * 50ms loop delay = 2 seconds for one complete warning cycle
        if (blink_counter > 40) {
            blink_counter = 0; // Restart the timer after one full cycle
        }

        // --- 5. LCD DISPLAY LOGIC ---

        if (state == READY) {
            sprintf(lcd_line1, "Press Start Btn "); // Tell the operator how to start production
            sprintf(lcd_line2, "to begin batch. "); // Second line of the starting message
        } 
        else if (state == COMPLETE) {
            sprintf(lcd_line1, "Production      "); // Show that production is complete
            sprintf(lcd_line2, "Complete!       ");
        } 
        else {
            // Applies to RUNNING and STOPPED states
            
            // If the reject rate is too high, show the warning for part of the cycle
            if (is_fault && (blink_counter < 20)) {
                sprintf(lcd_line1, "Quality Warning!"); // Warn the operator about poor product quality
            } else {
                if (state == STOPPED) {
                    sprintf(lcd_line1, "*** STOPPED *** "); // Show that production has been paused
                } else {
                    // Show accepted products, target and rejected products while running
                    sprintf(lcd_line1, "Acc:%02d/%02d Rej:%02d", accepted_count, TARGET_COUNT, rejected_count);
                }
            }
            
            sprintf(lcd_line2, "Efficiency: %3d%%", efficiency); // Show the current production efficiency
        }

        // Output the prepared information to the two LCD lines
        lcd.locate(0, 0); // Move cursor to the first LCD line
        lcd.printf("%s", lcd_line1); // Display the first line
        
        lcd.locate(0, 1); // Move cursor to the second LCD line
        lcd.printf("%s", lcd_line2); // Display the second line

        ThisThread::sleep_for(50); // 50ms delay before the main loop repeats
    }
}
