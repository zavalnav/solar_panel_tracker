#include "address_map_nios2.h"
#include "tones.h"
#include <stdio.h>
#include <time.h>
#define ON_CYCLES 0x1fffffff  /* Cycles to turn motor on for */

#define zero 0b0111111
#define one 0b0000110
#define two 0b1011011
#define three 0b1001111
#define four 0b1100110
#define five 0b1101101 
#define six 0b1111101
#define seven 0b0000111
#define eight 0b1111111
#define nine  0b1100111
#define A  0b1110111
#define B  0b1111100
#define C  0b0111001
#define D  0b1011110 
#define E  0b1111001
#define F  0b1110001

void updateSensor();
void updateHex(int channel0);
void updateMotor(int);
void compare();
void time_countdown(int);
void displayHex(int dig0, int dig1);
	
extern volatile int sensorLeft = 0; /* sensor 0*/
extern volatile int sensorPanel = 0; /* sensor 2 */
extern volatile int sensorRight = 0; /* sensor 1 */

volatile int * HEX3_HEX0_ptr = (int *) 0xFF200020; // HEX3_HEX0 address

volatile int * adc_base = (int *) ADC_BASE;
volatile int channel0;
volatile int channel1;

void interval_timer_isr( )
{
	printf("Timer interrupt handler\n");
	/* Writing to ADC_BASE is writing to cntrl register. Updates values */
	*adc_base = 0x1;
		
	/* Read from Channel0 */
	channel0 = *adc_base;
	printf("Channel 0: %x\n", channel0);

	/* Read from Channel1 */
	//channel1 = *(adc_base + 1);
	
	updateHex(channel0);
	updateSensor();	
	return;
}


void sensor_isr(){

	printf("In sensor interrupt handler \n");		
	// determine which sensor caused interrupt
	volatile int *sensor_base = (int *) JP1_BASE;
	
	*(sensor_base + 3) = 0xffffffff;

	/* check which sensor caused an interrupt */
	int edge_capture_reg = *(sensor_base+3);
	printf("out_before: Sensor base: %x\n", *sensor_base);

	// sensor1 is ready
	if(((*sensor_base >> 27)&0x1)  == 1){ /*check bit 27 */
		printf("sensor0 caused interrupt\n");
		printf("state: %x\n", *sensor_base) ;
		
		/* switch to value state, enable sensor 0 */
		*sensor_base = 0xfffffbff; 
		printf("Sensor base: %x\n", *sensor_base);

		sensorLeft = (*sensor_base >> 27) & 0xf;	
		// call compare

	}
	printf("here\n");
	// sensor2 is ready
	if(((*sensor_base >> 28)&0x1)  == 1){ /*check bit 28 */
		printf("sensor1 caused interrupt\n");		
		printf("state: %x\n", *sensor_base) ;
		
		/* switch to value state, enable sensor 1 */
		*sensor_base = 0xffffefff;
		sensorRight = (*sensor_base >> 27) & 0xf;
		
		// call compare
	}
	compare();
	/* Acknowledge interrupt */
	*(sensor_base + 3) = 0x0;	
	return; 
}

void compare(){
	printf("SensorPanel value: %d\n", sensorPanel);
	printf("SensorRight value: %d\n", sensorRight);
	printf("SensorLeft value: %d\n", sensorLeft);

	// if sensorPanel < sensorRight -> move right
	// if sensorPanel < sensorLeft -> move left
	// note: move right/left should take place in installments 
	if(sensorRight > sensorLeft)
		if(sensorPanel < sensorRight){
			/* move clockwise */
			updateMotor(1);
		}
		else{
			/* turn motor off */
			updateMotor(2);
		}
	else if (sensorRight <= sensorLeft){
		if(sensorPanel < sensorLeft){
			/* move counterclockwise */
			updateMotor(0);
		}
		else{
			/* turn motor off */
			updateMotor(2);
		}
	}
}


/* updateMotor function:
 * dir = 1: clockwise
 * dir = 0: counterclockwise
 * dir = 2: off
*/
void updateMotor(int dir)
{
	printf("Updating mtor, dir = %d\n", dir);
	volatile int * legoBase = (int *) 0xFF200060;
	*(legoBase+1) = 0x07f557ff;
	
	switch(dir){
		case 0:
			* legoBase = 0xfffffffe;
			break;
		case 1:
			* legoBase = 0xfffffffc;
			break;
		case 2:
			* legoBase = 0xffffffff;
			break;
	}
	// printf("updateMotor legoBase: %x \n", *legoBase);
	// printf("ON_CYCLES: %d\n", ON_CYCLES);
	
	time_countdown(ON_CYCLES);
	*(legoBase) = 0xffffffff; /* turn motor off */
	printf("timed out\n");
		
}

void time_countdown(int cycle){
	int i;
	for (i = 0; i < 50; i++){
		printf("counting down\n");
		int * timer_base = (int *)TIMER_2_BASE; /* use another timer, not one set for interrupts */
		*timer_base = 0x0;   						 /* clear the timer */
		*(timer_base + 2) = 0xffff;		 /* write lower half of timer */
		*(timer_base + 3) = 0xffff;			 /* write upper half of timer */
		*(timer_base + 1) = 0x4;					 /* start the timer */
		while(*(timer_base) == 0x2)					 /*still running*/
		/*busy wait*/{;}
	}
}

/* Reads value of sensor2 and updates pannel sensor */
void updateSensor()
{
	/* update panel sensor */
	printf("Updating panel sensor\n");

	int * legoBase = (int *) 0xFF200060;

	/* set direction for motors and sensors to output and sensor data register to inputs*/
	//*(legoBase+1) = 0x07f557ff;
	
	/* enable sensor 2, disable all motors*/
	*(legoBase) = 0xffffbfff;
	//*(legoBase) = 0xffffeffff;

	
	volatile int legoVal = *legoBase;
	printf("waitinf for sensor value to be ready \n");
	
	while (legoVal >> 15 & 0x1 != 0){
		printf("waiting for sensor value\n");
		printf("legoBase: %x\n", *legoBase);
		legoVal = *legoBase;
		/* Sensor value is not ready to be read */	
	}
	
	printf("panel sensor is ready to be read\n");
	/* Sensor is ready */
	sensorPanel = legoVal >> 27 & 0xf;
	printf("sensorPanel value: %d\n", sensorPanel);
	
	*legoBase = 0xffdfffff;
	return;
}

void updateHex(int input){
	
	/* TODO insert logic for large values: 2 hexes required */
	int value = 5 * input / 255;
	// int value = input;
	printf("value: %d\n", value);
	int dig0=0, dig1=0;
	
	if (value > 9) {
		dig1 = value / 10;
	} 
	dig0 = value % 10;		

	printf("DEBUG::(d0: %d,d1: %d)\n", dig0, dig1);
	displayHex(dig0, dig1);
	
	// * HEX3_HEX0_ptr = F;
}

void displayHex(int dig0, int dig1) {
	
	int num;

	if (dig1 == 0x0)
		num = zero;
	else if (dig1 == 0x1)
		num = one;		
	else if (dig1 == 0x2)
		num = two;		
	else if (dig1 == 0x3)
		num = three;		
	else if (dig1 == 0x4)
		num = four;		
	else if (dig1 == 0x5)
		num = five;		
	else if (dig1 == 0x6)
		num = six;		
	else if (dig1 == 0x7)
		num = seven;		
	else if (dig1 == 0x8)
		num = eight;		
	else if (dig1 == 0x9)
		num = nine;		
	
	printf("DEBUG::num: %x\n", num);
	printf("DEBUG::num<<7: %x\n", num<<7);
	if (dig0 == 0x0)
		num = (num << 7 ) | zero;
	else if (dig0 == 0x1)
		num = (num << 7 ) | one;
	else if (dig0 == 0x2)
		num = (num << 7 ) | two;
	else if (dig0 == 0x3)
		num = (num << 7 ) | three;
	else if (dig0 == 0x4)
		num = (num << 7 ) | four;
	else if (dig0 == 0x5)
		num = (num << 7 ) | five;
	else if (dig0 == 0x6)
		num = (num << 7 ) | six;
	else if (dig0 == 0x7)
		num = (num << 7 ) | seven;
	else if (dig0 == 0x8)
		num = (num << 7 ) | eight;
	else if (dig0 == 0x9)
		num = (num << 7 ) | nine;

	printf("DEBUG::after OR-ing: %x\n", num);

	*HEX3_HEX0_ptr = num;

}
