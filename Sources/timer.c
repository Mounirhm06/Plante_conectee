#include<stdio.h>
#include "stm32l4xx.h"
#include"timer.h"

void Timer_initialisation(void)
 {
	 RCC->APB1ENR1 |= (1<<0);/*Timer TIM2 enable*/
	 TIM2->PSC = 79;/*80Mhz/80 => 1MHz (un tick par 1us) , l'horloge est HSI(a cause du I2C)*/
	 TIM2->ARR = 9;/* overflow à 10us*/
	 TIM2->CR1 |=(1<<0);/*enable timer bit CEN*/
	 TIM2->SR &= ~(1<<0);/*renitialiser le flag UIF*/
 }

 void delay_us(int us_delay)
 {
	/* delay de us_delay*10us */
	TIM2->CNT = 0 ; /*remise a zero du compteur*/
	int i;
	for(i = 0;i<us_delay;i++){
		 while(!((TIM2->SR)& (1<<0))){
		 /*Attente de lever le flag UIF*/
		 }
		 TIM2->SR &= ~(1<<0);/*renitialiser le flag UIF*/
	}
 }


