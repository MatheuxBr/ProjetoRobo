#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>


#define VM1     PB6  // Pino de controle do motor 1
#define M1A     PB7  // Pino de direção do motor 1
#define M1B     PB5  // Pino de direção do motor 1
#define VM2     PD7  // Pino de controle do motor 2
#define M2A     PD6  // Pino de direção do motor 2
#define M2B     PD5  // Pino de direção do motor 2

#define CONTROLE  PD2 // Pino de controle do IR (Interruptor de interrupção externa)
#define LED1      PB0 // LED 1
#define LED2      PB1 // LED 2
#define LED3      PB2 // LED 3
#define LDR       PC0 // Pino para o sensor LDR
#define LASER     PD4 // Pino do laser

volatile uint32_t ULTIMOCODIGO = 0, ULTIMOCODIGOVALIDO = 0;
volatile uint8_t contador = 0;
volatile uint8_t laserState = 0;  // Estado do laser (0 = desligado, 1 = ligado)

void setup()
{
    // Definindo os pinos como saída
    DDRB = (1 << VM1) | (1 << M1A) | (1 << M1B) | (1 << LED1) | (1 << LED2) | (1 << LED3);
    DDRD = (1 << VM2) | (1 << M2A) | (1 << M2B) | (1 << LASER);
    DDRC = (1 << LDR); // LDR como entrada
    
    // Configurar o pino de controle do IR como entrada
    DDRD &= ~(1 << CONTROLE);
    
    // Ativar pull-up no pino CONTROLE (botão do controle IR)
    PORTD |= (1 << CONTROLE);
    
    // Inicializa o laser desligado
    PORTD &= ~(1 << LASER);
    
    // Habilitar interrupções externas para o controle IR
    EICRA |= (1 << ISC01);  // Interrupção na borda de descida do pino CONTROLE (PD2)
    EIMSK |= (1 << INT0);    // Habilitar interrupção externa INT0
    
    // Habilitar interrupções globais
    sei();
    
    // Configura temporizadores, se necessário (por exemplo, para controlar o laser)
    TCCR0A = 0;
    TCCR0B = (1 << CS02) | (1 << CS00);  // Prescaler 1024
    TIMSK0 = (1 << TOIE0);  // Ativar interrupção de overflow do timer 0
}

// Função de movimento para frente
void AndarFrente()
{
    PORTB &= ~(1 << M1B);  // M1B LOW
    PORTB &= ~(1 << M2B);  // M2B LOW
    PORTB |= (1 << M1A);   // M1A HIGH
    PORTD |= (1 << M2A);   // M2A HIGH
}

// Função de movimento para trás
void AndarTras()
{
    PORTB &= ~(1 << M1A);  // M1A LOW
    PORTD &= ~(1 << M2A);  // M2A LOW
    PORTB |= (1 << M1B);   // M1B HIGH
    PORTB |= (1 << M2B);   // M2B HIGH
}

// Parar os motores
void Parar()
{
    PORTB &= ~(1 << M1A) & ~(1 << M1B);  // M1A e M1B LOW
    PORTD &= ~(1 << M2A) & ~(1 << M2B);  // M2A e M2B LOW
}

// Função para giro (rodando em torno do próprio eixo)
void Giro()
{
    PORTB &= ~(1 << M2B);  // M2B LOW
    PORTB &= ~(1 << M1A);  // M1A LOW
    PORTB |= (1 << M1B);   // M1B HIGH
    PORTD |= (1 << M2A);   // M2A HIGH
}

// Função para ligar e desligar o laser a cada segundo
void alternarLaser()
{
    if (laserState)
    {
        PORTD &= ~(1 << LASER);  // Desliga o laser
    }
    else
    {
        PORTD |= (1 << LASER);   // Liga o laser
    }
    laserState = !laserState;
}

ISR(TIMER0_OVF_vect)
{
    static uint16_t count = 0;
    count++;
    if (count >= 1000) // Aproximadamente 1 segundo
    {
        alternarLaser(); // Alterna o estado do laser
        count = 0;
    }
}

// Interrupção para o controle IR
ISR(INT0_vect)
{
    // A leitura do código IR seria aqui, em um ambiente real você precisaria de um decodificador IR
    // Em vez disso, apenas simula um código IR
    ULTIMOCODIGO = 3924233868;  // Exemplo de código recebido
    if (ULTIMOCODIGO == 3924233868)
    {
        if (ULTIMOCODIGOVALIDO == 2673870592)
        {
            AndarFrente();
        }
        else if (ULTIMOCODIGOVALIDO == 2657158912)
        {
            Giro();
        }
        else if (ULTIMOCODIGOVALIDO == 2640447232)
        {
            // Andar para direita
        }
        else if (ULTIMOCODIGOVALIDO == 2590312192)
        {
            // Andar para esquerda
        }
        else
        {
            Parar();
        }
    }
    
    // Atualiza o código IR válido
    ULTIMOCODIGOVALIDO = ULTIMOCODIGO;
}

int main(void)
{
    setup();

    while (1)
    {
        // Loop principal
        uint16_t ldrleitura = ADC;  // Leitura do valor do LDR (ajustado para ADC)
        if (ldrleitura >= 700)
        {
            // Sequência de giro com LEDs
            if (contador == 0)
            {
                PORTB &= ~(1 << LED1);
                contador++;
                Giro();
                _delay_ms(800);
                Parar();
                _delay_ms(5000);
            }
            else if (contador == 1)
            {
                PORTB &= ~(1 << LED2);
                contador++;
                Giro();
                _delay_ms(800);
                Parar();
                _delay_ms(5000);
            }
            else if (contador == 2)
            {
                PORTB &= ~(1 << LED3);
                contador++;
                Giro();
                _delay_ms(800);
                Parar();
                _delay_ms(5000);
            }
        }

        _delay_ms(150);  // Pequeno atraso para evitar chamadas rápidas ao loop
    }
}
