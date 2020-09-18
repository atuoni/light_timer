/* 
 * File:   main.c
 * Author: Amauri Tuoni
 *
 * Created on 6 de Fevereiro de 2020, 16:22
 * 
 * Programa em linguagem C do Timer Light - (Luminária com Timer)
 * Versão: 1.1
 * 
 */

//********************************************************************************************************************************************************
// Ajuste dos bits de configuração do PIC12F675 
//
//Oscilador interno habilitado, watchdog desligado, power-up timer desabilitado, GP3 como I/O, Brown-out habilitado e codigos de proteção desabilitados
//********************************************************************************************************************************************************

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

#define _XTAL_FREQ 4000000         // frequencia do oscilador interno do MCU

#include <stdio.h>                   //biblioteca de manipulação de I/O
#include <stdlib.h>                  //biblioteca de proposito geral
#include <xc.h>                      //biblioteca com definição dos registradores


/*
 * 
 */


#define FALSE 0                 // define false como nível lógico "0"
#define TRUE 1                  // define true como nível lógico "1"
#define OFF 0                   // define off como nível lógico "0"
#define ON 1                    // define on como nível lógico "1"  
#define LIGA_LED 1              // define nível lógico "1" para LED Ligado
#define DESLIGA_LED 0           // define nível lógico "0" para LED Desligado

#define TIMER1_OVERFLOW 1                     // define o valor "1" como o estouro do TIMER 1 
#define TIMER1_RESET    0                        // define o valor "0" como o reset do TIMER 1
#define ANALOG_OFF 0b00000000                 // define todas as entradas como I/O
#define COMPARATOR_OFF 0b00000111             // define os comparadores do MCU desligados
#define CONFIG_IO 0b11001001                  // define as portas GP1,GP2,GP4 e GP5 como saída e GP3, GP0 como entrada
#define SEM_PULLUP 0b00000000                 // desliga os pull-ups de todos os I/Os


//*****************************************************************************************************************************************************************
// Ajuste dos tempos do timer para segundos --- (ajuste equivalente a 60 seguntos)
//*****************************************************************************************************************************************************************
#define TIMER_TEMPO_60SEG 114             // define o valor para um tempo de 60s (1min)  -> TIMER_TEMPO_60SEG = 60 s / TIMER1_OVERFLOW -> 60s/0,524s = 114,50 = 114



//*****************************************************************************************************************************************************************
//Define o valor para determinar os minutos no Timer
//*****************************************************************************************************************************************************************
#define TIMER_TEMPO_MINUTO 40          // TIMER_TEMPO_MINUTO = Tempo Desejado * 1 -> 40 * 60 = 2400s = 40 minutos
                                        

//****************************************************************************************************************************************
//Variáveis globais do programa
//****************************************************************************************************************************************

enum {modo_desligado=0,modo_timer,modo_sempre_ligado,modo_apagado};   // enumeração para definir os modos (estados) existentes no programa 
char modo_atual;              // variável utilizado para armazenar o modo (estado em que o programa está atualmente)
char timer_out;              // variavel utilizada para armazenar o estado do flag do Timer1 (overflow ou reset)
unsigned int count_button_press; //variavel 1 para contar o qtde do botão acionado
unsigned int count_button_press2;  //variavel 2 para contar a qtde do botão acionado
char primeira_vez_loop;          //variavel utilizada para armazenar se é a primeira vez que o loop do sempre ligado é realizado
    

//******************************************************************************************************************************************************** 
//Protótipos de funções do programa
//*********************************************************************************************************************************************************

void config_osc(void);
void config_mcu(void);
void config_var(void);
void mode_on(void);
void read_mode(void);
void modo_lowpower(void);
void config_Timer1(void);

void __interrupt() isr(void);   // função de interrupção

//*****************************************************************************************************************************************
//Função principal(main loop)
//*****************************************************************************************************************************************
            
int main(void) 
{
    config_osc();            // chama sub rotina de calibração do oscilador do MCU
    config_var();            // chama sub-rotina de inicialização das variaveis do programa
    config_Timer1();         // chama sub-rotina de configuração do Timer 1
    config_mcu();            // chama sub-rotina de configuração dos periféricos do MCU
    
    while (1)      //loop infinito
    {               
        mode_on();    // executa rotinas de acordo com o modo_atual do programa
    }    
        
    return (0);       //fim do programa principal
}

//*******************************************************************************************************************************
// Função para calibrar o oscilador interno -OSCCAL
//
//*******************************************************************************************************************************

void config_osc(void)
{
     __asm ("BSF STATUS, 5");       //  Seta o bit 5 do registrador STATUS 
     __asm ("CALL 3FFh");          // salta para a última posição da memória Flash 
     __asm ("MOVWF OSCCAL");      // move o valor do registrador W para o resgistrador OSCCAL
     __asm ("BCF STATUS, 5");     // Zera o bit 5 do registador STATUS           
}

//*******************************************************************************************************************************
//Função para configurar os perifericos do MCU
//*******************************************************************************************************************************

void config_mcu(void)
{
    ANSEL = ANALOG_OFF;            // desliga as entradas analógicas
    CMCON = COMPARATOR_OFF;        //desliga os comparadores
    TRISIO = CONFIG_IO;            // configura os ports GP1,GP2,GP4 e GP5 como saída
    WPU = SEM_PULLUP;              // configura os pull_up de I/O desligados
    GP1 = OFF;                     // não utilizado nesse programa  
    GP2 = OFF;                     // desliga Led
    GP4 = OFF;                     // não utilizado nesse programa
    GP5 = OFF;                     // não utilizado nesse programa
    GIE = ON;                      // interrupção global ligada
    GPIE = ON;                     // interrupção de mudança de nível em I/O ligada
    IOC0 = ON;                      // interrupção de mudança de nível em GP0 ligada
    PEIE = ON;                      // interrupção dos perifericos ligada 
   
}

//*********************************************************************************************************************
// Inicialização das variáveis iniciais do programa
//*********************************************************************************************************************

void config_var(void)
{
   count_button_press = 0;           // inicializa o contador de botão pressionado como 0 
   modo_atual = modo_desligado;      //inicializa o programa no modo_desligado (led apagado)
   timer_out = TIMER1_RESET;         // inicializa a variável de fim de contagem contagem do timer 1 
   primeira_vez_loop = FALSE;        // inicializa a variável que determina se eh a primeira vez no loop sempre_ligado da subrotina mode_on
                                     // como falso
}

//***************************************************************************************************************************
//Função para configuração do TIMER 1
//
//ciclo de maq = 4MHz/4 = 1MHz   1MHz = 1us
//t=ciclo de maq * prescaler * contagem(65536-TMRIH:TMR1L) = 1us * 8 *(65536-0) = 524288us = 524,3 ms = 0,524 s -->1,91Hz
//***************************************************************************************************************************

void config_Timer1(void)
{
    T1CON = 0b00110000;         // Configura o registrador T1CON do TIMER1( Oscilador Inteno, Prescaler 1:8, Timer Desligado, sincronia com o CLK)
    TMR1IF = OFF;               // limpa flag de overflow do Timer 1 
    TMR1L = 0x00;               // Zera a parte baixa do Timer 1
    TMR1H = 0x00;               // Zera a parte alta  do Timer 1
    TMR1IE = ON;                // Liga Interrupção para o Timer 1
    TMR1ON = OFF;               // Desliga a contagem do Timer1
}

//***************************************************************************************************************************************
// Função tratar cada estado do programa conforme o estado (modo) selecionado
//***************************************************************************************************************************************


void mode_on(void)
{

    unsigned int timer_count_seg;  // variavel utilizada para armazenar o valor dos segundos
    unsigned int timer_count_min;  // variavel utilizada para armazenar o valor dos minutos
    
 
    if(modo_atual == modo_timer)                  // caso modo_atual estiver no modo_timer
    {    
        timer_count_seg = TIMER_TEMPO_60SEG;      // carrega a variavel com o valor referente a 60 segundos = 1 minuto
        timer_count_min = TIMER_TEMPO_MINUTO;     // carrega a variavel com o valor referente qtde de minutos desejada
                
        TMR1ON = ON;                              // liga a contagem do Timer 1
             
       
        while(timer_count_min != 0 && modo_atual == modo_timer)     //loop while - verificando se contagem de minutos acabou e se estado atual está em modo_timer 
        {
                       
            GP2 = LIGA_LED;                                         // aciona Led de iluminação
            TMR1ON = ON;                                            // liga o Timer 1 para contagem
            if(timer_out == TIMER1_OVERFLOW)                        // verifica estouro do timer
            {
                        
                timer_count_seg--;                                  // decrementa a qtde de segundos
                timer_out = TIMER1_RESET;                           // reseta flag de estouro do timer1
                TMR1ON = ON;                                        // liga timer1 para contagem novamente
                
                if(timer_count_seg == 0)                            // verifica se qtde de segundos é igual a 0
                {
                    timer_count_seg = TIMER_TEMPO_60SEG;            // carrega novamente a qtde referente a 60segundos
                    timer_count_min--;                              // decrementa a quantidade de minutos
                }              
            } 
                      
            if(timer_count_min == 1 && timer_count_seg == TIMER_TEMPO_60SEG)         //verifica se o contador de minuto está no ultimo minuto
            {
                GP2 = DESLIGA_LED;                                                   //desliga o led de iluminação por 500ms (alerta de ultimo minuto)
                __delay_ms(500);
                GP2 = LIGA_LED;                                                      //liga o led novamente
            }
                      
        }                                                //fim do while
        if(modo_atual != modo_sempre_ligado)             //verifica se o modo atual foi para o modo sempre ligado (sai do loop modo_timer)
        {
            modo_atual = modo_apagado;                   //define o modo atual para o modo apagado no fim da contagem do modo timer
        }
    }                 
    if(modo_atual == modo_sempre_ligado)                 //verifica se o modo atual está em modo sempre ligado
    {
        TMR1ON = OFF;                                    //desliga a contagem do timer1
        timer_out = TIMER1_RESET;                        //reseta do flag de estouro do timer1               
        if (primeira_vez_loop == TRUE)                   //verifica se é a primeira vez dentro do loop do modo sempre ligado
        {
            GP2 = DESLIGA_LED;                           // desliga led de iluminação por 1segundo
            __delay_ms(1000);                            
            primeira_vez_loop = FALSE;                   // configura que o loop foi atingido pela primeira vez
        }
            GP2 = LIGA_LED;                              // liga o led de iluminação (agora sempre ligado)
               
    }       
        
    if( modo_atual == modo_apagado)                     //verifica se o modo atual está em modo apagado
    {           
        timer_out = TIMER1_RESET;                       //reseta o flag de estouro do timer1
        TMR1ON = OFF;                                   //desliga a contagem do Timer1
        GP2= DESLIGA_LED;                               //desliga o led de iluminação
        while(GP0 == OFF);                              //loop infinito enquanto o botão estiver pressionado                          
        __delay_ms(100);                                // aguarda 100ms quando o botão for despressionado    
        if(GP0 == ON)                                   // se o botão for solto (nivel alto)
        {       
            timer_count_min=0;                          //inicializa a contagem de minutos
            timer_count_seg=0;                          //inicializa a contagem de segundos
            modo_atual=modo_desligado;                  //define o modo atual como modo desligado
        }
                
    }      
        
    if(modo_atual == modo_desligado)                   // verifica se o modo atual está em modo desligado
    {
      __delay_ms(3000);                                 // atraso de 3s
      modo_lowpower();                                 //entra no modo low power
    }
}


//***************************************************************************************************************************************
// Função para definir o modo atual (estado) de acordo com o pressionamento do botão
//***************************************************************************************************************************************
   
void read_mode(void)
{
    count_button_press2 = 0;  // inicializa com 0 a qtde de acionamentos do botão
    
    switch(modo_atual)        // switch para definir o modo atual
    {
        case modo_desligado:     // caso em modo desligado entra no modo_timer                           
                                count_button_press = 0;
                                modo_atual = modo_timer;   //define o modo atual como modo timer
                                __delay_ms(1000);          //atraso de 1s para iniciar o modo timer
                                break;                     //sai do case
                                   
        case modo_timer:        // caso em modo timer entra no modo sempre ligado
                                while(GP0==OFF && count_button_press2<=100)     //enquanto o botao estiver pressionado e for menor igual a 100
                                {
                                    count_button_press++;                       // incrementa a variavel enquanto o botão estiver acionado
                                    if(count_button_press == 1000)              // se atingir o valor valor 1000
                                    { 
                                        count_button_press2 ++;                 // incrementa a variavel2
                                        count_button_press=0;                   // limpa a variavel 1
                                    }
                                } 
                                if(count_button_press2 >= 100)                  // se a variavel 2 for maior igual a 100    
                                {                                               
                                    modo_atual = modo_sempre_ligado;            // define o modo atual como modo sempre ligado  
                                    primeira_vez_loop = TRUE;                   // define a primeira vez dentro do loop como verdadeiro
                                }
                                count_button_press = 0;                         // reseta a variavel 1
                                count_button_press2 = 0;                        // reseta a variavel 2
                                break;                                          // sai do case
                                  
                
        case modo_sempre_ligado:    // caso em modo sempre ligado entra no modo apagado
                                    while(GP0==OFF && count_button_press2 <=100)  //enquanto o botao estiver pressionado e for menor igual a 100
                                    {
                                        count_button_press++;                     // incrementa a variavel enquanto o botão estiver acionado
                                        if(count_button_press == 1000)            // se atingir o valor 1000
                                        {
                                            count_button_press2++;                // incrementa a variavel2
                                            count_button_press=0;                 // limpa a variavel 1
                                        }
                                    }  
                                    if(count_button_press2 >= 100)                //se a variavel 2 for maior igual a 100
                                    {
                                        modo_atual = modo_apagado;                //define o modo atual como modo apagado
                                    }
                                    count_button_press = 0;                       // reseta a variavel 1
                                    count_button_press2 = 0;                      // reseta a variavel 2
                                    break;                                        // sai do case
                                          
               /* case modo_apagado:  
                                             modo = modo_desligado;
                                             count_button_press = 0;
                                             //GP1=0;
                                             break;
                */                    
                  
    }
}

//*******************************************************************************************************************************
// Função para acionamento do modo low power
//*******************************************************************************************************************************
void modo_lowpower(void)
{
    SLEEP();                   // entra no modo de low power
    NOP();                     // aguarda 2 ciclos de clock para sair do modo de low power 
    NOP();
}

//*******************************************************************************************************************************
//Vetor de interrupção - Interrupção de mudança de estado em GP0 (botão) e estouro do TIMER 1
//*******************************************************************************************************************************

void __interrupt() isr(void){         //sub-rotina de tratamento de interrupções
   

//Interrupção de mudança de estado em I/O
//*****************************************************************************************************************************    
    
    if (GPIE == ON && GPIF == ON)     //verifica se a interrupção do Timer ocorreu GPIF=ON (botão acionado)
        
    {
        GPIE = OFF;                 // desliga a interrupção geral das portas 
       
        if (GP0 == OFF)             // verifica se o botão está acionado
        {
              read_mode();          // chama sub-rotina que determina o estado atual (modo atual)
        }      
              GPIE = ON;            // liga a interrupcao geral das portas
              GPIF = OFF;           // reset o flag de interrupcao da mudança de estado I/O
    }
    
//Interrupção do Timer 1
//*****************************************************************************************************************************    

    
    if(TMR1IE == ON && TMR1IF== ON)              // verifica se a interrupção do timer1 e o flag do Timer 1 foi acionado
    {                                            
        timer_out = TIMER1_OVERFLOW;             // overflow do timer 1
        TMR1ON = OFF;                            // desliga o Timer 1
        TMR1L = 0x00;                            // zera o registrador "baixo" de contagem do Timer1
        TMR1H = 0x00;                            // zera o registrador "alto" de contagem do Timer1
        TMR1IF = OFF;                            // reset o flag do timer1  
    }
    
}  // fim da rotina de tratamento de  interrupcao