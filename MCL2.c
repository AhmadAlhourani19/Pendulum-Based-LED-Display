#include <inc/tm4c1294ncpdt.h>
#include <stdio.h>
#include <stdint.h>
#define MAX_SIZE 15


void wait(unsigned long time);

/**
 * main.c
 */
int main(void)
{
    /*Array of LED patterns*/
    int i = 0; // array indexer
    int a[MAX_SIZE] = { 0x00,0x00,0x00,0xFF,0xFF,0xFF,0x18,0x18,0x18,0xFF,0xFF,0xFF,0x00,0x00,0x00 }; // task 3.2 letter H
    //int a[MAX_SIZE] = {0x00,0xFF,0xFF,0x18,0xFF,0xFF,0x00,0x81,0xFF,0x81,0x00,0x81,0xFF,0x81,0x00}; // task 3.3 letters H I I

    /*Configure port L(0) as input*/
    SYSCTL_RCGCGPIO_R |= (1 << 10); // enable clock port L (number 10, 0x400)
    while (!(SYSCTL_PRGPIO_R & 0x400)); // wait before accessing port

    GPIO_PORTL_DEN_R = 0x01; // enable L(0)
    GPIO_PORTL_DIR_R = 0x00; // set as input

    /*Configure port M(7:0) as outputs*/
    SYSCTL_RCGCGPIO_R |= (1 << 11); // enable clock port M (number 11, 0x800)
    while (!(SYSCTL_PRGPIO_R & 0x800)); // wait before accessing port

    GPIO_PORTM_DEN_R = 0xFF; // enable M(7:0)
    GPIO_PORTM_DIR_R = 0xFF; // set as outputs

    /*Configure timer 0A and 0B*/
    SYSCTL_RCGCTIMER_R |= (1 << 0); // system clock on timer 0
    while (!(SYSCTL_PRTIMER_R & 0x01)); // wait for timer 0 activation
    TIMER0_CFG_R = 0x04; // 16 bit mode
    TIMER0_CTL_R &= ~0x0101; // disable timer 0A and 0B by clearing bits 0 and 8

    TIMER0_TAMR_R = 0x02; // periodic mode, match disabled
    TIMER0_TAPR_R = 1 - 1; // prescaler of 1 for a 4ms timer duration which enables 15 LED changes in 60ms
    TIMER0_TAILR_R = 64000 - 1; // ILR of Timer 0A of 64000

    TIMER0_TBMR_R = 0x01; // one-shot mode, match disabled
    TIMER0_TBPR_R = 6 - 1;// Timer 0B prescaler of 6 for a 22.5ms timer duration, to start LED patterns in centre
    TIMER0_TBILR_R = 60000 - 1;// ILR of Timer 0B of 60000

    while (1)
    {
        /*Wait until we are more centrally located using timer 0B*/
        TIMER0_CTL_R |= 0x100; // enable timer 0B
        while ((TIMER0_RIS_R & (1 << 8)) == 0); // wait for time-out of timer 0B	
        TIMER0_ICR_R |= (1 << 8); // clear time-out flag
        TIMER0_CTL_R &= ~0x100; // disable timer 0B

        /*Start displaying LED patterns using timer 0A*/
        TIMER0_CTL_R |= 0x0001; // enable timer 0A by setting bit 0 to 1
        while ((GPIO_PORTL_DATA_R & 0x01) == 1) // left to right transition
        {
            GPIO_PORTM_DATA_R = a[i];
            while ((TIMER0_RIS_R & (1 << 0)) == 0); // wait for time-out
            TIMER0_ICR_R |= (1 << 0); // clear time-out flag
            if (i >= MAX_ARRAY - 1) // once all LED patterns have been iterated through
            {
                TIMER0_CTL_R &= ~0x0001; // disable timer 0A
                break; // escape while-loop
            }
            else
            {
                i++;
            }
        }

        while ((GPIO_PORTL_DATA_R & 0x01) == 1); // wait for left to right transition to finish

        /*Wait until we are more centrally located using timer 0B*/
        TIMER0_CTL_R |= 0x100; // enable timer 0B
        while ((TIMER0_RIS_R & (1 << 8)) == 0); // wait for time-out of timer 0B	
        TIMER0_ICR_R |= (1 << 8); // clear time-out flag
        TIMER0_CTL_R &= ~0x100; // disable timer 0B

        /*Start displaying LED patterns using timer 0A*/
        TIMER0_CTL_R |= 0x0001; // enable timer 0A by setting bit 0 to 1
        while ((GPIO_PORTL_DATA_R & 0x01) == 0) // right to left transition
        {
            GPIO_PORTM_DATA_R = a[i];
            while ((TIMER0_RIS_R & (1 << 0)) == 0); // wait for time-out
            TIMER0_ICR_R |= (1 << 0); // clear time-out flag
            if (i <= 0) // once all LED patterns have been iterated through
            {
                TIMER0_CTL_R &= ~0x0001; // disable timer 0A
                break; // escape while-loop
            }
            else
            {
                i--;
            }
        }

        while ((GPIO_PORTL_DATA_R & 0x01) == 0); // wait for right to left transition to finish
    }
}
