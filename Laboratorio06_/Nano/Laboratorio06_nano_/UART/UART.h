/*
 * UART.h
 *
 * Created: 9/3/2026 5:49:09 PM
 *  Author: ayala
 */ 

/*
 * UART.h
 *
 * Created: 7/23/2026 6:15:35 PM
 *  Author: ayala
 */ 


#ifndef UART_H_
#define UART_H_

#include <stdint.h>
#include <avr/io.h>

extern volatile uint8_t uart_valor;
extern volatile uint8_t uart_nuevo;

void Init_UART(void);					// inicializar UART
void UART_enviar(char c);
void write_str(const char *texto);		
void enviar_numero(uint16_t num);		// Enviar num decimal

uint8_t UART_dato_disponible(void);
void UART_procesar_entrada(void);

uint8_t UART_recibir(void);

#endif /* UART_H_ */

