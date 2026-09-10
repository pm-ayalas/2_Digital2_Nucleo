/*
 * Laboratorio06_nano_.c
 *
 * Created: 9/3/2026 5:29:46 PM
 * Author : ayala
 */ 


#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include "UART/UART.h"

// ========== DEFINICIONES ==========
// Para 5 ms con prescaler 1024 y F_CPU = 16 MHz:
#define T1Valor 77

// Umbral de rebote: 10 muestras * 5 ms = 50 ms
#define DEBOUNCE_THRESHOLD  10

// ========== VARIABLES GLOBALES ==========
volatile uint8_t Bandera_BTN0 = 0;	// Derecha
volatile uint8_t Bandera_BTN1 = 0;	// Izquierda
volatile uint8_t Bandera_BTN2 = 0;	// Arriba
volatile uint8_t Bandera_BTN3 = 0;	// Abajo
volatile uint8_t Bandera_BTN4 = 0;	// Opción A
volatile uint8_t Bandera_BTN5 = 0;	// Opción B

volatile uint8_t debounced_PC0 = 0;
volatile uint8_t debounce_counter_PC0 = 0;
volatile uint8_t debounced_PC1 = 0;
volatile uint8_t debounce_counter_PC1 = 0;
volatile uint8_t debounced_PC2 = 0;
volatile uint8_t debounce_counter_PC2 = 0;
volatile uint8_t debounced_PC3 = 0;
volatile uint8_t debounce_counter_PC3 = 0;
volatile uint8_t debounced_PC4 = 0;
volatile uint8_t debounce_counter_PC4 = 0;
volatile uint8_t debounced_PC5 = 0;
volatile uint8_t debounce_counter_PC5 = 0;

// ========== PROTOTIPOS ==========
void setup(void);
void init_TMR1(void);

// ========== MAIN ==========
int main(void)
{
    cli();
    setup();
    init_TMR1();
    Init_UART();
    sei();

    write_str(":)\r\n");

    while (1)
    {
        if (Bandera_BTN0) {
            write_str("DERECHA\r\n");
            Bandera_BTN0 = 0;   // limpia bandera
        }
        if (Bandera_BTN1) {
            write_str("IZQUIERDA\r\n");
            Bandera_BTN1 = 0;
        }
        if (Bandera_BTN2) {
            write_str("ARRIBA\r\n");
            Bandera_BTN2 = 0;
        }
        if (Bandera_BTN3) {
            write_str("ABAJO\r\n");
            Bandera_BTN3 = 0;
        }
        if (Bandera_BTN4) {
            write_str("OPCION A\r\n");
            Bandera_BTN4 = 0;
        }
        if (Bandera_BTN5) {
            write_str("OPCION B\r\n");
            Bandera_BTN5 = 0;
        }
    }
}


void setup(void)
{


    // Deshabilitar UART 
    UCSR0B = 0x00;

    // PC0-PC5 como entradas con pull-up
    DDRC &= ~((1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3) | (1 << PC4) | (1 << PC5));
    PORTC |= (1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3) | (1 << PC4) | (1 << PC5);

    // Deshabilitar interrupción de cambio de pin
    PCICR &= ~(1 << PCIE1);
    PCMSK1 = 0;
}

void init_TMR1(void)
{
    TCCR1B = 0;
    TCCR1B |= (1 << WGM12);        // CTC
    TCCR1B |= (1 << CS12) | (1 << CS10); // Prescaler 1024 (CS12 y CS10)
    OCR1A = T1Valor;
    TIMSK1 |= (1 << OCIE1A);       // Habilitar interrupción por comparación
    TCNT1 = 0;
}

// INTERRUPCIÓN DEL TIMER1 (cada 5 ms) 
ISR(TIMER1_COMPA_vect)
{
    // Lectura y Antirrebote Btn0
    uint8_t raw_PC0 = (PINC >> PC0) & 1;
    if (raw_PC0 == debounced_PC0) {
        debounce_counter_PC0 = 0;
    } else {
        if (debounce_counter_PC0 < 255) {
            debounce_counter_PC0++;
        }
        if (debounce_counter_PC0 >= DEBOUNCE_THRESHOLD) {
            debounced_PC0 = raw_PC0;
            if (debounced_PC0 == 1) {
                Bandera_BTN0 = 1;
            }
            debounce_counter_PC0 = 0;
        }
    }
	
	// Lectura y Antirrebote Btn1
	uint8_t raw_PC1 = (PINC >> PC1) & 1;  
	if (raw_PC1 == debounced_PC1) {
		debounce_counter_PC1 = 0;
		} else {
		if (debounce_counter_PC1 < 255) {
			debounce_counter_PC1++;
		}
		if (debounce_counter_PC1 >= DEBOUNCE_THRESHOLD) {
			debounced_PC1 = raw_PC1;
			if (debounced_PC1 == 1) {
				Bandera_BTN1 = 1;   // activar bandera
			}
			debounce_counter_PC1 = 0;
		}
	}
	
	// Lectura y Antirrebote Btn2
	uint8_t raw_PC2 = (PINC >> PC2) & 1;
	if (raw_PC2 == debounced_PC2) {
		debounce_counter_PC2 = 0;
		} else {
		if (debounce_counter_PC2 < 255) {
			debounce_counter_PC2++;
		}
		if (debounce_counter_PC2 >= DEBOUNCE_THRESHOLD) {
			debounced_PC2 = raw_PC2;
			if (debounced_PC2 == 1) {
				Bandera_BTN2 = 1;   // activar bandera
			}
			debounce_counter_PC2 = 0;
		}
	}
	
	// Lectura y Antirrebote Btn3
	uint8_t raw_PC3 = (PINC >> PC3) & 1;
	if (raw_PC3 == debounced_PC3) {
		debounce_counter_PC3 = 0;
		} else {
		if (debounce_counter_PC3 < 255) {
			debounce_counter_PC3++;
		}
		if (debounce_counter_PC3 >= DEBOUNCE_THRESHOLD) {
			debounced_PC3 = raw_PC3;
			if (debounced_PC3 == 1) {
				Bandera_BTN3 = 1;   // activar bandera
			}
			debounce_counter_PC3 = 0;
		}
	}
	
    // Lectura y Antirrebote Btn4
    uint8_t raw_PC4 = (PINC >> PC4) & 1;   // estado actual del pin

    // Filtro de rebotes (con contador)
    if (raw_PC4 == debounced_PC4) {
        // Estado estable: reiniciar contador
        debounce_counter_PC4 = 0;
    } else {
        // Cambio detectado: incrementar contador
        if (debounce_counter_PC4 < 255) {
            debounce_counter_PC4++;
        }
        // Si se supera el umbral, considerar el cambio como válido
        if (debounce_counter_PC4 >= DEBOUNCE_THRESHOLD) {
            // Actualizar estado estabilizado
            debounced_PC4 = raw_PC4;
            // Detectar flanco de subida (0 -> 1) (según tu lógica original)
            if (debounced_PC4 == 1) {
                Bandera_BTN4 = 1;   // activar bandera
            }
            // Reiniciar contador para evitar múltiples disparos
            debounce_counter_PC4 = 0;
        }
    }
	
	// Lectura y Antirrebote Btn5
	uint8_t raw_PC5 = (PINC >> PC5) & 1;
	if (raw_PC5 == debounced_PC5) {
		debounce_counter_PC5 = 0;
		} else {
		if (debounce_counter_PC5 < 255) {
			debounce_counter_PC5++;
		}
		if (debounce_counter_PC5 >= DEBOUNCE_THRESHOLD) {
			debounced_PC5 = raw_PC5;
			if (debounced_PC5 == 1) {
				Bandera_BTN5 = 1;   // activar bandera
			}
			debounce_counter_PC5 = 0;
		}
	}

}

