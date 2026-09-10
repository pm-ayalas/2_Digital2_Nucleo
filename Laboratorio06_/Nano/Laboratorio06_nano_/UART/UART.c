/*
 * UART.c
 *
 * Created: 9/3/2026 5:48:58 PM
 *  Author: ayala
 */ 
/*
 * UART.c
 *
 * Created: 7/23/2026
 * Author: ayala
 */

#include "UART.h"
volatile uint8_t uart_valor = 0;
volatile uint8_t uart_nuevo = 0;

static uint16_t numero_parcial = 0;
static uint8_t recibiendo_numero = 0;

void Init_UART(void)
{
    // 9600 baudios @16 MHz
    UBRR0H = 0;
    UBRR0L = 103;

    // Habilitar transmisor y receptor
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);

    // 8 bits, sin paridad, 1 bit de parada
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_enviar(char c)
{
    while (!(UCSR0A & (1 << UDRE0)));

    UDR0 = c;
}

void write_str(const char *texto)
{
    while (*texto)
    {
        UART_enviar(*texto++);
    }
}

void enviar_numero(uint16_t num)
{
    char buffer[6];
    uint8_t idx = 0;

    if(num == 0)
    {
        UART_enviar('0');
        return;
    }

    while(num > 0)
    {
        buffer[idx++] = (num % 10) + '0';
        num /= 10;
    }

    while(idx)
    {
        UART_enviar(buffer[--idx]);
    }
}

uint8_t UART_recibir(void)
{
    while (!(UCSR0A & (1 << RXC0)));

    return UDR0;
}

uint8_t UART_dato_disponible(void)
{
    return (UCSR0A & (1 << RXC0));
}

void UART_procesar_entrada(void)
{
    if(!UART_dato_disponible())
        return;

    char c = UART_recibir();

    if(c >= '0' && c <= '9')
    {
        uint16_t nuevo = numero_parcial * 10 + (c - '0');

        if(nuevo <= 255)
        {
            numero_parcial = nuevo;
            recibiendo_numero = 1;
        }
    }
    else if(c == '\r' || c == '\n')
    {
        if(recibiendo_numero)
        {
			uart_valor = (uint8_t)numero_parcial;
			uart_nuevo = 1;

            numero_parcial = 0;
            recibiendo_numero = 0;
        }
    }
    else
    {
        numero_parcial = 0;
        recibiendo_numero = 0;
    }
}