#include<stdio.h>
#include"stm32l4xx.h"
#include"capteur_temperature.h"
#include"timer.h"


/*on utilise le pin PA0 pour communiquer avec le capteur*/

void pin_output_config(void)
{
	/*configurer PA0 en sortie*/

	__HAL_RCC_GPIOA_CLK_ENABLE();// 1. Activer l'horloge du port GPIOA
    // 2. Configurer le pin PA0
	GPIO_InitTypeDef GPIO_InitStruct; //creer une structure qui contient les champs du pin PA0
	GPIO_InitStruct.Pin = GPIO_PIN_0; //choisir le pin P0
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // Mode sortie push-pull
	GPIO_InitStruct.Pull = GPIO_NOPULL;          // Pas de pull-up ni pull-down
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; // Vitesse lente
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); // 3. Initialiser le pin avec la configuration

}

void pin_input_config(void)
{
	/*configurer PA0 en entree*/

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;     // Mode entrée
	GPIO_InitStruct.Pull = GPIO_NOPULL;          // Pas de pull-up ni pull-down
	//GPIO_InitStruct.Pull = GPIO_PULLUP; // Activer la résistance de pull-up
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

void capteur_mesures(uint8_t mesures[])
{
	/*cette fonction permet de remplir le tableau mesures  avec les mesures recues du capteur*/

	uint8_t i,j;
		for(i = 0 ; i < 5 ; i++){
		/*parcourir tout les octet envoyes par le capteur*/
			for (j=0;j<8;j++){
				/*parcourir les bits de chaque octet*/
				while (!(HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_0)));   // attente de la mise a 1 du PA0 (front montant)
				delay_us (4);   // attente de 40us

				if (!(HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_0))){   /*si le pin est a 0 */
							mesures[i]&= ~(1<<(7-j));   // ecrire 0 (reception d'un 0)
						}
				else{
					mesures[i]|= (1<<(7-j));  // ecrire 1 (reception d'un 1)
				}

				while (HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_0));  // attente de front decendent pour passer au bit suivant
			}
		}
}




