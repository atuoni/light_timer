/* 
 * File:   main.c
 * Author: amauri
 *
 * Created on 6 de Fevereiro de 2020, 16:22
 */


// PIC12F675 Configuration Bit Settings
// 'C' source line config statements
// CONFIG
#pragma config FOSC = INTRCIO   // Oscillator Selection bits (INTOSC oscillator: I/O function on GP4/OSC2/CLKOUT pin, I/O function on GP5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-Up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // GP3/MCLR pin function select (GP3/MCLR pin function is digital I/O, MCLR internally tied to VDD)
#pragma config BOREN = ON       // Brown-out Detect Enable bit (BOD enabled)
#pragma config CP = OFF         // Code Protection bit (Program Memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#define _XTAL_FREQ 4000000 

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>


/*
 * 
 */


#define FALSE 0                 // define false como nível lógico "0"
#define TRUE 1                  // define true como nível lógico "1"
#define OFF 0                   // define off como nível lógico "0"
#define ON 1                    // define on como nível lógico "1"  
#define LIGA_LED 0              // define nível lógico "1" para LED Ligado
#define DESLIGA_LED 1           // define nível lógico "0" para LED Desligado
#define LIGA_RELE 1             // define nível lógico "1" para Relé Ligado
#define DESLIGA_RELE 0          // define nível lógico "0" para Relé Desligado
#define LIGA_BUZZER 1           // define nível lógico "1"  para Buzzer Ligado
#define DESLIGA_BUZZER 0           // define nível lógico "0" para Buzzer Desligado
#define BOTAO_SEM_TOQUE 0          // define o valor "0" para nenhum toque do botão
#define BOTAO_PRIMEIRO_TOQUE 1     // define o valor "1" para o primeiro toque do botão
#define BOTAO_SEGUNDO_TOQUE 2      // define o valor "2" para o segundo toque do botão
#define CICLO_COMPLETO_TIMER 0     // define o valor "0" para o ciclo completo do timer 
#define USER_CANCELA 1             // define o valor "1" para o caso o usuário cancela
#define TOQUES_BUZZER_FIM 15       // define a quantidade de toques do buzzer para o ciclo completo
#define TOQUES_BUZZER_CANCELA 3    // define a quantidade de toques do buzzer qdo o ciclo é cancelado pelo usuário
#define CONT_LEDS_PISCA 60         // define a quantidade de piscadas dos leds vermelho ou verde - configurado: 1 minuto = 60s ( aprox. 1 piscada/s) 
#define CONT_BT_PRESSIONADO 15       // define o valor limite do contador para o botão pressionado - configurado: 15 = 15s ( 15 piscadas até ativar)       
#define MODO_LED_VERMELHO 0x00          // define o modo 1 (led vermelho acionado) como 0x00
#define MODO_LED_VERDE 0xFF             // define o modo 2 (led verde acionado) como 0xFF
#define TIMER1_OVERFLOW 1                     // define o valor "1" como o estouro do TIMER 1 
#define TIMER1_RESET 0                        // define o valor "0" como o reset do TIMER 1
#define ANALOG_OFF 0b00000000                 // define as entradas como I/O
#define COMPARATOR_OFF 0b00000111             // define os comparadores desligados
#define CONFIG_IO 0b11001001                  // define as portas GP1,GP2,GP4 e GP5 como saída e GP3 e GP0 entrada
#define SEM_PULLUP 0b00000000                 // desliga os pull-ups dos I/Os


//*****************************************************************************************************************************************************************
// Ajuste dos tempos de preparo (CARGA LIGADA)
//*****************************************************************************************************************************************************************
#define TIMER_TEMPO_60SEG 115             // define o valor para um tempo de 60s (1min)  -> TIMER_TEMPO_60SEG = 60 s / TIMER1_OVERFLOW -> 60s/0,524s = 114,50 = 115

//*****************************************************************************************************************************************************************
//Define o valor para um tempo em minutos no modo1 - MODO LED VERMELHO
//*****************************************************************************************************************************************************************
#define TIMER_TEMPO_MINUTO 3           // TEMPO_TIMER_VERMELHO = Tempo Desejado * 1 -> 5 * 60 = 300s = 5 minutos
                                        

//****************************************************************************************************************************************
//Variáveis globais
//****************************************************************************************************************************************

enum {modo_desligado,modo_timer,modo_sempre_ligado}; 
char modo;
char timer_out;               // variavel utilizada para armazenar o modo de encerramento do modo de preparo (CICLO COMPLETO OU CANCELAMENTO PELO USUÁRIO)
char count_button_press;
//******************************************************************************************************************************************************** 
//Prototipos de funções
//*********************************************************************************************************************************************************

void config_osc(void);
void config_mcu(void);
void config_var(void);
void timer_on(void);
void read_button(void);
//char estado_botao(void);
//void cooking(char tempo);
//void modo_lowpower(void);
void config_TM1(void);
//void som_modo_preparo(char modo);
//void som(void);
void __interrupt() isr(void);

//*****************************************************************************************************************************************
//Função principal(loop)
//*****************************************************************************************************************************************
            
int main(void) {

    config_osc();            // chama sub rotina de calibração do oscilador do MCU
    config_mcu();            // chama sub-rotina de configuração dos periféricos do MCU
    config_var();            // chama sub-rotina de inicialização das variaveis do programa
    config_TM1();            // chama sub-rotina de configuração do Timer 1
   
    while (1)
    {               // loop infinito
         
        read_button();
        if(modo == modo_timer)
        {
            timer_on();
        }
             
    }    
        
    
    return (EXIT_SUCCESS);
}

//*******************************************************************************************************************************
// Função para calibrar o oscilador interno -OSCCAL 
//*******************************************************************************************************************************

void config_osc(void){
__asm("BSF STATUS, 5");       //  Seta o bit 5 do registrador STATUS 
__asm ("CALL 3FFh");          // salta para a última posição da memória Flash 
__asm  ("MOVWF OSCCAL");      // move o valor do registrador W para o resgistrador OSCCAL
__asm  ("BCF STATUS, 5");     // Zera o bit 5 do registador STATUS           
}


//*******************************************************************************************************************************
//Função para configurar os perifericos do MCU
//*******************************************************************************************************************************

void config_mcu(void){
    ANSEL = ANALOG_OFF;            // desliga as entradas analógicas
    CMCON = COMPARATOR_OFF;        //desliga os comparadores
    TRISIO = CONFIG_IO;            // configura os ports GP0,GP1,GP2,GP4 e GP5 como saída
    WPU = SEM_PULLUP;              // configura os pull_up de I/O desligados
    GP0 = OFF;                     // desliga relé 
    GP1 = OFF;                     // desliga buzzer   
    GP2 = OFF;                     // I/O não utilizado nesse progama
    GP4 = OFF;             // apaga o led vermelho
    GP5 = OFF;             // apaga o led verde   
    GIE = ON;                      // interrupção global ligada
    PEIE= ON;                      // interrupção dos perifericos ligada 
    GPIE = ON;                     // interrupção de mudança de nível em I/O ligada
    IOC0= ON;                      // interrupção de mudança de nível em GP0 ligada
   
}

//*********************************************************************************************************************
//Incialização das variáveis iniciais do programa
//*********************************************************************************************************************

void config_var(void){
   count_button_press = 0;           // inicializa o contador de botão pressionado 
   modo = modo_desligado;
  //  cont_pisca_leds = 0;            // inicializa o contador de piscadas dos leds vermelho ou verde
  //  modo_timer = MODO_LED_VERMELHO;              // inicializa como modo 1 (Led vermelho)   
  //  bt_estado_atual = FALSE;        // inicializa o estado atual do botão como não pressionado
  //  bt_estado_anterior = FALSE;     // inicializa o estado anterior do botão como não pressionado
  //  bt_num_toques = BOTAO_SEM_TOQUE;   // inicializa o status do numero de toques no botão com nenhum toque 
  //  cont_toques = 0;                  // inicializa o contador de toques no botão para "0"
    timer_out = TIMER1_RESET;         // inicializa a variável de fim de contagem contagem do timer 1 
}

//***************************************************************************************************************************
//Função para configuração do TIMER 1
//
//ciclo de maq = 4MHz/4 = 1MHz   1MHz = 1us
//t=ciclo de maq * prescaler * contagem(65536-TMRIH:TMR1L) = 1us * 8 *(65536-0) = 524288us = 524,3 ms = 0,524 s -->1,91Hz
//***************************************************************************************************************************
void config_TM1(void){
    T1CON = 0b00110000;         // Configura o registrador T1CON do TIMER1( Oscilador Inteno, Prescaler 1:8, Timer Desligado, sincronia com o CLK)
    TMR1IE = ON;                // Liga Interrupção para o Timer 1
    TMR1L = 0x00;               // Zera a parte baixa do Timer 1
    TMR1H = 0x00;               // Zera a parte alta  do Timer 1
}


void timer_on(void){
    int timer_cont_seg;
    int timer_cont_min;
    
    timer_cont_min = TIMER_TEMPO_MINUTO;
    timer_cont_seg = TIMER_TEMPO_60SEG;
    TMR1ON = ON;
    GP2 = ON;
    while(timer_cont_min > 0)
    { 
        if(timer_out == TIMER1_OVERFLOW)
        {
            //read_button();
            timer_cont_seg--;
            timer_out = TIMER1_RESET;
            TMR1ON=ON;
            
                if(timer_cont_seg == 0)
                {
                        timer_cont_seg = TIMER_TEMPO_60SEG;
               
                        timer_cont_min--;
                        if(timer_cont_min == 1)
                        {
                            GP2 = OFF;
                            __delay_ms(500);
                            GP2 = ON;
                        }
               
                }
        }
        
       
        
    }    
    TMR1ON = OFF;
    GP2 = OFF;
    modo = modo_desligado;
} 


void read_button(void)
{
    if(GP0 != ON)
    { 
        __delay_ms(10);
        if(GP0 != ON)
        {
        
            switch(modo){
                case modo_desligado:{
                                        modo = modo_timer;
                                        break;
                                    }
                case modo_timer: {
                                       count_button_press++;
                                       if(count_button_press > 5)
                                       {
                                           modo = modo_sempre_ligado;
                                           count_button_press = 0;
                                       }
                                       break;
                                 } 
                
                case modo_sempre_ligado: {
                                            count_button_press++; 
                                            if(count_button_press > 50)
                                            {
                                                modo = modo_desligado;
                                                count_button_press = 0;
                                            }
                                            break;
                                          } 
       
            }
        }
    }    
}




//*******************************************************************************************************************************
//Vetor de interrupção - Interrupção de mudança de estado em GP3 e estouro do TIMER 1
//*******************************************************************************************************************************

void __interrupt() isr(void){   //sub-rotina de tratamento de interrupções
   

//Interrupção de mudança de estado em I/O
//*****************************************************************************************************************************    
    
    if ((GPIE && GPIF) == ON){                 // verifica se o flag de mudança de nível em I/O foi acionado
        if (GP0 == ON || GP0 == OFF){        // lê o estado de GP0
        GPIF = OFF;                          // reset no flag de mudança de nível
        }
    }
    

//Interrupção do Timer 1
//*****************************************************************************************************************************    

    
    if(TMR1IE == ON && TMR1IF== ON)
    {                                            // verifica se a interrupção do timer1 e o flag do Timer 1 foi acionado
        timer_out = TIMER1_OVERFLOW;             // seta a variavel timer_out
        TMR1IF = OFF;                            // reset o flag do timer1
        TMR1ON = OFF;
        TMR1L = 0x00;                            // zera o registrador "baixo" de contagem do Timer1
        TMR1H = 0x00;                            // zera o registrador "alto" de contagem do Timer1
    }
    
}