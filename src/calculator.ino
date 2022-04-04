//--! Credit, Copyright & Info
/*

	Copyright (c) 2022 - Hazardev

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is furnished
	to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
	PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
	OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.



	A simple calculator thrown together with leftover parts
	Powered by Arduino using C

	- Created and coded by Sam (Hazardev#0001)
	- Mathematic parsing done w/ tinyexpr
	- Hardware by Sam and Tom



	If you want to try and use this;
	
	- Follow schematic for wiring:  - https://img.hazardev.com/x/1K9Bs7ANAUiZvgwi.png
									- https://img.hazardev.com/x/BV0Nfyqs1UWPmMgD.png
	
	- Change header pins if wired slightly differnetly, each of which you can find commented below
	  in the pin settings (advanced) section!

	- Upload it straight to arduino AS IS after changing the above

*/


//--! Headers
#include "tinyexpr.h"
#include <LiquidCrystal.h>
#include <string.h>
#include <stdlib.h>



//--! Globals
// settings
#define MAX_RESISTANCE 1023
#define MAX_EQUATION 128
#define GAP_EQUATION 3

#define BOMB_SECONDS 45
#define BOMB_SPEEDUP 1
#define BOMB_MAXSTEP 11

#define LCD_MAX_X_SIZE 16
#define LCD_MAX_Y_SIZE 2

// pin settings (advanced)
const int buzz = 7;														//  This pin determines the piezo buzzer pin
const int sign_in = A1, num_in = A0;									//  These are the potentiometer pins (they must use analog pins [A0 - A5])
const int sign_button = 9, num_button = 13, result_button = 8;			//  These are all of the button pins

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;				//  These are the pins that determine the LCD inputs

// lcd consts
const int dimx = LCD_MAX_X_SIZE, dimy = LCD_MAX_Y_SIZE;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// calculation consts
const int sign_amount = 9;
const char sign_enum[sign_amount] = {
	'%', '^', '(', ')', '/', '*', '-', '+', '.'
};

// status vars
int sign_button_status = 0, num_button_status = 0, result_button_status = 0;
int sign_action = 0, num_action = 0, result_action = 0;
char num_buffer = '0';
char sign_buffer = sign_enum[0];

int cursor_index = 0;
int equation_index = 0;
char displaying[dimy][dimx];
char display_buffer[dimy][dimx];
char equation_buffer[MAX_EQUATION];

int initd = 0;



//--! Prototypes
void error(char text[(dimx * dimy) - 8]);
void calculate();
const char *parse_equation();
void continue_index();
void add_sign();
void add_num();

int get_resistance_num(int r);

void clear_display_buffer();
void update_display_buffer();
void clear_display();
void update_display();
void clear_garbage();

void switch_sound();
void button_sound();
void success_sound();
void fail_sound();

void end();

void check_easter_eggs();
void meaning_of_life();
void csgo_bomb();



//--! Required
void setup()
{
	// initialize
	lcd.begin(dimx, dimy);
	Serial.begin(9600);

	// set pinmodes
	pinMode(buzz, OUTPUT);
	pinMode(sign_button, INPUT_PULLUP);
	pinMode(num_button, INPUT_PULLUP);
	pinMode(result_button, INPUT_PULLUP);

	// clear
	clear_garbage();
	clear_display();
	clear_display_buffer();

	// initd
	initd = 1;
}


void loop()
{	
	// wait for initd
	while (initd == 0)
	{
		delay(50);
	}

	// update button states
	sign_button_status = digitalRead(sign_button);
	num_button_status = digitalRead(num_button);
	result_button_status = digitalRead(result_button);

	// update buffers
	int num = get_resistance_num(analogRead(num_in), 10);
	char new_num_buffer = (char) ((int) num + (int) '0');

	int sign = get_resistance_num(analogRead(sign_in), sign_amount);
	char new_sign_buffer = (char) sign_enum[sign];

	// check buffers
	if (new_num_buffer != num_buffer)
	{
		switch_sound();
		num_buffer = new_num_buffer;
	}

	if (new_sign_buffer != sign_buffer)
	{
		switch_sound();
		sign_buffer = new_sign_buffer;
	}

	// check button press
	if (num_button_status == LOW && num_action == 0)
	{
		button_sound();
		add_num();
		num_action = 1;
	}
	else if (num_button_status == HIGH)
	{
		num_action = 0;
	}

	if (sign_button_status == LOW && sign_action == 0)
	{
		button_sound();
		add_sign();
		sign_action = 1;
	}
	else if (sign_button_status == HIGH)
	{
		sign_action = 0;
	}

	if (result_button_status == LOW && result_action == 0)
	{
		check_easter_eggs();
		calculate();
		result_action = 1;
	}
	else if (result_button_status == HIGH)
	{
		result_action = 0;
	}

	// update display buffers
	display_buffer[dimy - 1][dimx - 1] = sign_buffer;
	display_buffer[0][cursor_index] = num_buffer;
	display_buffer[1][cursor_index] = '^';
	display_buffer[1][cursor_index - 1] = ' ';

	// update display
	update_display();
}



//--! Functions
void error(char *text)
{
	lcd.clear();

	char *pretext = "Error :";
	int length = strlen(text);
	int prelength = strlen(pretext);

	for (int i = 0; i < prelength; i++)
	{
		lcd.setCursor(i, 0);
		lcd.write(pretext[i]);
	}

	for (int i = 0; i < strlen(text); i++)
	{
		lcd.setCursor(i, 1);
		if (i == dimx - 1)
		{
			lcd.write('.');
		}
		else
		{
			lcd.write(text[i]);
		}		
	}

	fail_sound();
	end();
}


void calculate()
{
	// parse and reformulate to string
	const char *equ = parse_equation();

	// tinyexpr parser
	int err;
	double ans = (double) te_interp(equ, &err);

	// error check
	if (err != 0)
	{
		free((void *) equ);
		error("Invalid math");
	}
	else
	{
		char output[MAX_EQUATION];

		dtostrf(ans, 0, 10, output);

		display_buffer[0][cursor_index] = ' ';
		display_buffer[dimy - 1][dimx - 1] = ' ';
		display_buffer[dimy - 1][0] = '=';

		for (int i = 0; i < dimx - 1; i++)
		{	
			if (output[i] != '\0')
			{
				display_buffer[dimy - 1][i + 1] = output[i];
			}
		}

		update_display();
		success_sound();
		end();
	}
}


const char *parse_equation()
{
	char *equ = (char *) calloc(MAX_EQUATION, sizeof(char));
	for (int i = 0; i < MAX_EQUATION; i++)
	{
		char b = equation_buffer[i];
		if (b == ' ' || b == '\0' || b == '\n')
		{
			break;
		}
		else
		{
			equ[i] = equation_buffer[i];
		}
	}
	
	return equ;
}


void continue_index()
{
	equation_index++;
	if (cursor_index < dimx - GAP_EQUATION)
	{
		cursor_index++;
	}
}


void add_sign()
{
	if (equation_index < MAX_EQUATION)
	{
		equation_buffer[equation_index] = sign_buffer;
		continue_index();
		update_display_buffer();
	}
}


void add_num()
{
	if (equation_index < MAX_EQUATION)
	{
		equation_buffer[equation_index] = num_buffer;
		continue_index();
		update_display_buffer();
	}
}


void clear_display_buffer()
{
	for (int y = 0; y < dimy; y++)
	{
		for (int x = 0; x < dimx; x++)
		{
			display_buffer[y][x] = ' ';
		}
	}
}


void update_display_buffer()
{
	int max_size = dimx - GAP_EQUATION;
	if (cursor_index < max_size)
	{
		for (int i = 0; i < max_size; i++)
		{
			display_buffer[0][i] = equation_buffer[i];
		}
	}
	else
	{
		display_buffer[0][1] = ' ';
		display_buffer[0][0] = '<';
		for (int i = 2; i < max_size; i++)
		{
			display_buffer[0][i] = equation_buffer[equation_index - max_size + i];
		}
	}
}


void clear_display()
{
	lcd.clear();
	for (int y = 0; y < dimy; y++)
	{
		for (int x = 0; x < dimx; x++)
		{
			displaying[y][x] = ' ';
		}
	}
}


void update_display()
{
	for (int y = 0; y < dimy; y++)
	{
		for (int x = 0; x < dimx; x++)
		{
			char k = display_buffer[y][x];
			char chk = displaying[y][x];
			if (k != chk)
			{
				lcd.setCursor(x, y);
				lcd.write(k);
				displaying[y][x] = k;
			}
		}
	}
}


int get_resistance_num(int r, int max)
{
	for (int i = 0; i < max; i++)
	{
		int comp = (int)(((double) MAX_RESISTANCE / (double) max) * (double)(i + 1));
		if (r <= comp)
		{
			return i;
		}
	}
}


void clear_garbage()
{
	for (int y = 0; y < dimy; y++)
	{
		for (int x = 0; x < dimx; x++)
		{
			displaying[y][x] = '\255';
			display_buffer[y][x] = ' ';
		}
	}

	for (int i = 0; i < MAX_EQUATION; i++)
	{
		equation_buffer[i] = ' ';
	}
}


void switch_sound()
{
	tone(buzz, 100, 50);
}


void button_sound()
{
	tone(buzz, 350, 75);
}


void success_sound()
{
	tone(buzz, 325, 75);
	delay(75);
	tone(buzz, 460, 75);
	delay(75);
	tone(buzz, 600, 75);
}


void fail_sound()
{
	tone(buzz, 600, 80);
	delay(80);
	tone(buzz, 300, 95);
}


void end()
{
	for (;;)
	{
		delay(1000);
	}
}



//--! Easter Eggs :)
void check_easter_eggs()
{
	// csgo easter egg
	char *bomb = "7355608";
	for (int i = 0; i < 7; i++)
	{
		if (equation_buffer[i] == bomb[i])
		{
			if (i == 7 - 1)
			{
				csgo_bomb();
			}
		}
		else
		{
			break;
		}
	}

	// meaning of life easter egg
	char *mof = "42";
	for (int i = 0; i < 2; i++)
	{
		if (equation_buffer[i] == mof[i])
		{
			if (i == 2 - 1)
			{
				meaning_of_life();
			}
		}
		else
		{
			break;
		}
	}
}


void meaning_of_life()
{
	lcd.clear();
	lcd.home();
	lcd.print("The meaning     ");
	lcd.setCursor(0, 1);
	lcd.print("        of life.");
	end();
}


void csgo_bomb()
{
	lcd.clear();
	lcd.home();
	lcd.print("    PLANTING    ");

	for (int i = 0; i < 5; i++)
	{
		tone(buzz, 1400, 140);
		delay(160);
	}

	delay(1200);

	int inc = 1;
	char pretimer[16];
	sprintf(pretimer, "    00'%02d'00    ", BOMB_SECONDS);

	lcd.clear();
	lcd.home();
	lcd.print(pretimer);

	for (int s = BOMB_SECONDS - 1; s >= 0; s--)
	{
		for (int ms = 99; ms >= 0; ms = ms - inc)
		{
			char timer[16];
			sprintf(timer, "    00'%02d'%02d    ", s, ms);

			lcd.home();
			lcd.print(timer);

			lcd.setCursor(0, 1);
			if (ms >= 90 || ms < 2)
			{
				lcd.print("  ------------  ");
			}
			else
			{
				lcd.print("                ");
			}

			double speed = 8;
			if (BOMB_SPEEDUP == 1)
			{
				double mult = (((double) s * 100) + ms) / ((double) BOMB_SECONDS * 100);
				speed = speed * mult;
				inc = (int) floor(((double) 1 - mult) * (double) BOMB_MAXSTEP);
				if (inc < 1) { inc = 1; }
			}

			delay((int) floor(speed));
		}

		tone(buzz, 1400, 140);
	}
	
	lcd.clear();
	lcd.home();
	lcd.print("    00'00'00    ");
	tone(buzz, 1400, 2000);

	lcd.setCursor(0, 1);
	lcd.print(" **** BOOP **** ");

	end();
}