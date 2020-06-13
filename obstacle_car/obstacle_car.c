/*
 * obstacle_car.c
 *
 * Created: 19/05/2020 12:42:56 AM
 *  Author: Mohamed Elsayed
 */ 
#include "UART.h"
#include "LCD.h"
#include "PWM.h"
#include "Timer0.h"
void timer_on();						//Turn on timer when we trigger the ultrasonic sensor
void timer_off();						//Turn off timer when we receive echo of the ultrasonic sensor
void sensor_trig();						//Send pulse to trigger the sensor
void get_display_distance();			//Calculate and display distance on lcd
uint16_t  total_time=0 , distance=0 ;
uint8_t timer_over_flow=0;				
uint8_t PWM=0;							//percentage of PWM from 0% to 100%
uint8_t receiver=0;						//Received signal from Bluetooth
uint8_t flag=0;							//Determine status of the car moving forward , backward ,right or left
int main(void)
{
	CLRBIT(MCUCSR,ISC2);				// External interupt enable for falling edge 
	sei();								// global interrupt enable
	LCD_init();							// initializing LCD
	Uart_init();						// initializing UART
	timer_normal_init();				// initializing timer 0 in normal mode 
	PWM_init_timer1_oc1a();				// initializing timer 1 in PWM mode (OC1A)
	PWM_init_timer1_oc1b();				// initializing timer 1 in PWM mode (OC1A)
	set_duty_oc1a(PWM);					// set duty value
	set_duty_oc1b(PWM);					// set duty value
	SETBIT(DDRC,2);						// Will send pulse from PORT C pin 2 to trigger sensor
    SETBIT(DDRC,0);						//left Motor
	SETBIT(DDRC,1);						//left Motor
	SETBIT(DDRC,3);						//Right Motor
	SETBIT(DDRC,4);						//Right Motor
	uint8_t starting=0;					//Check for first turn on 
    while(1)
    {
		if (starting==0){
			SETBIT(PORTC,0);			//Turn Left motor ON
			SETBIT(PORTC,3);			//Turn Right motor ON
			starting=1;
		}
		timer_on();						
		sensor_trig();
			
		if(distance<=400&&distance>10){  //that mean no obstacle exist
		set_duty_oc1b(PWM);
		set_duty_oc1a(PWM);
		if (flag==0){					//Moving forward or Left
		SETBIT(PORTC,3);
		CLRBIT(PORTC,4);
		}
		else if (flag==1){				//Moving backward
			SETBIT(PORTC,4);
			CLRBIT(PORTC,3);
		}
		else if (flag==2){				//Moving Right
			CLRBIT(PORTC,4);
			CLRBIT(PORTC,3);
		}
				} 
		if(distance<=10&&distance>=0){   // There is an obstacle so we will turn right 
			CLRBIT(PORTC,3);
			CLRBIT(PORTC,4);
				
				}
		
		 _delay_ms(250);    
		     }
}
ISR(INT2_vect){							//External interrupt INT2 (Echo)
	CLRBIT(GICR,INT2);
	total_time=TCNT0*1024.0 /16.0;
	total_time=total_time+timer_over_flow*(16230);   // 16230us = 255*1024/16 refers to the ovf_value_for_timer0
	timer_off();
	timer_over_flow=0;
	get_display_distance();
		}
ISR(TIMER0_OVF_vect){
	timer_over_flow++;
	}
void timer_on(){
	TCNT0=0;
	SETBIT(TCCR0,CS02);
	SETBIT(TCCR0,CS00);
	SETBIT(GICR,INT2);
}
void timer_off(){
	CLRBIT(TCCR0,CS02);
	CLRBIT(TCCR0,CS00);	
}
void get_display_distance(){
	distance=0.01715*total_time;		// 2x=34300*t & to get time in seconds we multiplied by 10^-6
	distance=distance-10;
	LCD_write_command(0x1);
	LCD_write_string("distance=");
	LCD_write_num(distance);
	LCD_write_string("cm");
	if (distance<=10){
	LCD_write_command(0xc0);
	LCD_write_string("Danger!!!");
	}
	}
void sensor_trig(){
		SETBIT(PORTC,2);
		_delay_us(10); // according to ultrasonic data sheet 10 us pulse will be good to trigger the sensor
		CLRBIT(PORTC,2);
		/*
		c.c = (1/16Mhz)
		time t represent time of starting wave of ultrasonic sensor 
		TCNT0 multiplied by 1023 the brescaler and c.c to know the real time
		*/
}

ISR(USART_RXC_vect){
	receiver=UDR;
		if(receiver=='F'){			//Forward
			flag=0;
			PWM=80;
			CLRBIT(PORTC,1);
			CLRBIT(PORTC,4);
			SETBIT(PORTC,0);
			SETBIT(PORTC,3);
			set_duty_oc1a(PWM);
			set_duty_oc1b(PWM);
			Uart_Write_string(" F to Move forward & B to Move backward & R to Move right & L to Move left & O to turn off ");
			}
		else if(receiver=='B'){		//Backword
			flag=1;
			PWM=80;
			CLRBIT(PORTC,0);
			CLRBIT(PORTC,3);
			SETBIT(PORTC,1);
			SETBIT(PORTC,4);
			set_duty_oc1a(PWM);
			set_duty_oc1b(PWM);
			}
		else if(receiver=='R'){		//Right
			flag=2;
			PWM=80;
			CLRBIT(PORTC,0);
			SETBIT(PORTC,1);
			CLRBIT(PORTC,3);
			CLRBIT(PORTC,4);
			set_duty_oc1a(PWM);
			set_duty_oc1b(0);	
		}
		else if(receiver=='L'){		//Left
			flag=0;
			PWM=80;
			CLRBIT(PORTC,0);
			CLRBIT(PORTC,1);
			SETBIT(PORTC,3);
			CLRBIT(PORTC,4);
			set_duty_oc1a(0);
			set_duty_oc1b(PWM);
		}
		else if(receiver=='O'){		//Turn off
			PWM=0;
			CLRBIT(PORTC,0);
			CLRBIT(PORTC,1);
			CLRBIT(PORTC,3);
			CLRBIT(PORTC,4);
			set_duty_oc1a(PWM);
			set_duty_oc1b(PWM);
		}
		else if(receiver=='Q'){		//Quick
			PWM=90;
			set_duty_oc1a(PWM);
			set_duty_oc1b(PWM);
		}
		else if(receiver=='S'){		//Slow
			PWM=70;
			set_duty_oc1a(PWM);
			set_duty_oc1b(PWM);
		}
    }

	

