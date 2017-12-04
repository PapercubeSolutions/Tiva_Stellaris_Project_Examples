/*
 PROGRAMA: VALIDA ANGULOS SERVOMOTOR
 Autor: RKZ
 DESCRIPCION: Este programa recibe �ngulos via hyperterminal y valida si estan entre 0� y 180�
 */

#include <stdint.h>
#include "tm4c123gh6pm.h"

void config_UART1 (void){
	// Habilitamos reloj para el UART0
	SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART0;
	// Habilitamos reloj para GPIOA
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA;
	// Inhabilitamos el UART0
	UART0_CTL_R &= ~UART_CTL_UARTEN;
	// Velocidad 9600bps (Fsysclk = 16MHz)
	UART0_IBRD_R = (UART0_IBRD_R & 0xFFFF0000) | 104;
	UART0_FBRD_R = (UART0_FBRD_R & 0xFFFFFFC0) | 11;
	// 8, N, 1, FIFOs habilitados
	UART0_LCRH_R = (UART0_LCRH_R & 0xFFFFFF00) | 0x70;
	// Habilitamos el UART0
	UART0_CTL_R |= UART_CTL_UARTEN;
	// Desactivamos modo anal�gico en PA0 y PA1
	GPIO_PORTA_AMSEL_R &= ~(0x03);
	// Conectamos UART0 a PA0 y PA1
	GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFFFFFF00)|0x00000011;
	// Activamos funciones alternas en PA0 y PA1
	GPIO_PORTA_AFSEL_R |= 0x03;
	// Activamos funciones digitales en PA0 y PA1
	GPIO_PORTA_DEN_R |= 0x03;
	}

void config_sw(void){
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
	while( (SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R5)==0) { }
	GPIO_PORTF_DIR_R &= ~(0x10); // PF4 y PF0 pin de entrada
	GPIO_PORTF_AFSEL_R &= ~(0x10); // no usamos funci�n alternativa
	GPIO_PORTF_PUR_R |= 0x10; // activamos resistencia de pull-up en pin PF4
	GPIO_PORTF_DEN_R |= 0x10; // PF4 pin digital
}

/*Funci�n para la transmisi�n de un car�cter por UART1*/
void txcar_uart1(uint32_t car){
	//Espera disponibilidad para transmitir
	while(UART0_FR_R & UART_FR_TXFF);
	// Se escribe el dato para su transmisi�n
	UART0_DR_R = car;
}

/*Funci�n para la recepci�n de un car�cter de 8 bits por UART1*/
uint8_t rxcar_uart1(void){
	uint8_t temp;
	while(UART0_FR_R & UART_FR_RXFE);
	// Se lee el dato recibido
	temp = UART0_DR_R;
	return temp;
}

/*Funci�n para el envio de una cadena*/
void txmens(uint8_t mens[]){
	uint8_t letra;
	uint8_t i=0;
	letra= mens[i++];
	while (letra != '\0'){ //Se env�an todos los caracteres hasta el fin de cadena
		txcar_uart1(letra);
		letra= mens[i++];
		}
}

int rxnumero(uint8_t arreglo[10]){
	uint8_t letra;
	uint8_t i=0;
	int valor=0;
	letra = rxcar_uart1();
	arreglo[i]=letra;
	while (letra != 13){ //Se leen todos los caracteres hasta el enter
		valor=letra-48+valor*10;
		letra=rxcar_uart1();
		i++;
		arreglo[i]=letra;
	}
	return valor;
}

void envianumero(int numero ){
	uint8_t c;
	uint8_t d;
	uint8_t u;
	c=(int)numero/100;
	if(c!=0){txcar_uart1(c+48);}
	numero=numero%100;
	d=(int)numero/10;
	if(d!=0){txcar_uart1(d+48);}
	if((d==0)&&(c!=0)){txcar_uart1(d+48);}
	numero=numero%10;
	u =(int)numero;
	if(u!=0){txcar_uart1(u+48);}
	if((u==0)&&(c!=0)){txcar_uart1(u+48);}
	if((u==0)&&(d!=0)){txcar_uart1(u+48);}
}

void main(void) {
	config_UART1();
	config_sw();
	uint8_t arreglo[10];
	int grados;
	
	while(1){
		while((GPIO_PORTF_DATA_R& 0x10) == 0x10) //Espera que sea presionado SW1
		while((GPIO_PORTF_DATA_R& 0x10) == 0)//Espera que sea soltado	SW1
		txmens("Introduzca el angulo a girar: \n\r");
		grados =rxnumero(arreglo);
		if((grados>=0)&&(grados<181)){
			txmens("Moviendo a angulo de ");
			envianumero(grados);
			txmens("\n\r");
		}
		else
		{	txmens("Angulo invalido: ");
			txmens("\n\r");
		}
	}
}
