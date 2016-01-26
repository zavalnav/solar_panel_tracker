#include "address_map_nios2.h"
#include "nios2_ctrl_reg_macros.h"
#include "key_codes.h"
#include <stdio.h>




int main(void)
{
	
	/* Declare volatile pointers to I/O registers (volatile means that IO load
	 * and store instructions will be used to access these pointer locations, 
	 * instead of regular memory loads and stores)
	*/
	

	volatile int * interval_timer_ptr = (int *) TIMER_BASE;	// interal timer base address
	volatile int * legoBase = (int *) JP1_BASE;	// interal timer base address
	volatile int * adc_base = (int *) ADC_BASE;
	volatile int channel0;
	volatile int channel1;

	int counter = 400000;				
	*(interval_timer_ptr + 0x2) = (counter & 0xFFFF);
	*(interval_timer_ptr + 0x3) = (counter >> 16) & 0xFFFF;

	/* start interval timer, enable its interrupts */
	*(interval_timer_ptr + 1) = 0x7;	// STOP = 0, START = 1, CONT = 1, ITO = 1 
	
	/* Turn switch 0 and switch 1 on */
	//*legoBase = 0xffffebff;
	
		
	/* set motor,threshold and sensors bits to output, set state and sensor valid bits to inputs */
	*(legoBase + 1) = 0x07f557ff;
	
	
	/* Turn everything off */
	//*legoBase = 0xffffffff;
	
	/* switch to state mode */ 
	*legoBase = 0xffdfffff;
		
	
	/* 
	 * enable sensor interrupt for sensor0 ans sensor1 
	*/
	*(legoBase + 2) =  0x18000000;
	
	NIOS2_WRITE_IENABLE( 0x809 );		/* set interrupt mask bits for IRQ0 (timer) IRQ 3(switch) IRQ11(JP1) */

	NIOS2_WRITE_STATUS( 1 );			// enable Nios II interrupts
	
	printf("HERE\n");
	

	while(1){
		
	}								

}
