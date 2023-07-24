#include <DMDESP.h>
#include <Arduino.h>
#include <fonts/Arial_black_16.h>

#define Display_High 1
#define Display_Wide 5
#define Brightness 1
DMDESP dmd(Display_Wide, Display_High);

enum status{
	start = 0,
	stop,
	reset,
	idle,
	busy,
};

enum seq {
	PA = 0b0001,
	PB = 0b0010,
	PC = 0b0100,
	PD = 0b1000,
	PAB = 0b0011,
	PCD = 0b1100,
	PABCD = 0b1111,
};

#define RED dmd.drawFilledRect(0,0,31,16);
#define YELLOW dmd.drawFilledRect(32,0,63,16);
#define GREEN dmd.drawFilledRect(128,0,159,16);

#define printA dmd.drawText(66,1,"A");
#define printB dmd.drawText(84,1,"B");
#define printC dmd.drawText(98,1,"C");
#define printD dmd.drawText(117,1,"D");

void select_player(int player)
{
	if(player&1)
		printA;
	if((player>>1)&1)
		printB;
	if((player>>2)&1)
		printC;
	if((player>>3)&1)
		printD;
}

static uint32_t lastmillis = 0;
int duration = -1;
int countdown = -1;
int start_duration = 10;

int timer = reset;
int status = idle;
int player=PABCD;

void setup()
{
	Serial.begin(9600); // 74880
	dmd.start();
	dmd.setBrightness(Brightness);
	dmd.setFont(Arial_Black_16);
	select_player(player);
	RED
}

void get_timer(String _input)
{
	if (_input == "A")
		duration = 30;
	else if (_input == "B")
		duration = 60;
	else if (_input == "C")
		duration = 90;
	else if (_input == "E")
		duration = 120;
	else if (_input == "F")
		duration = 150;
	else if (_input == "G")
		duration = 180;
}

void get_seq(String _input)
{
	if (_input == "M")
	{
	if(player&PA)
		player &= (~PA);
	else
		player |= (PA);
	}
	else if (_input == "N")
	{
	if(player&PB)
		player &= (~PB);
	else
		player |= (PB);
	}
	else if (_input == "O")
	{
	if(player&PC)
		player &= (~PC);
	else
		player |= (PC);
	}
	else if (_input == "P")
	{
	if(player&PD)
		player &= (~PD);
	else
		player |= (PD);
	}
	else if (_input == "I")
	{
	if(player&PAB)
		player &= (~PAB);
	else
		player |= (PAB);
	}
	else if (_input == "J")
	{
	if(player&PCD)
		player &= (~PCD);
	else
		player |= (PCD);
	}
	else if (_input == "K")
	{
	if(player&PABCD)
		player &= (~PABCD);
	else
		player |= (PABCD);
	}
}

void loop()
{
	dmd.loop();
	if (status == idle){
		while(status==idle){
			dmd.refresh();
			if (Serial.available()){
				char input = char(Serial.read());
				String _input = String(input);
				_input.trim();

				if(_input == "A" || _input == "B" || \
					_input == "C" || _input == "E" || \
					_input == "F" || _input == "G"){ //timer

					get_timer(_input);

					countdown = duration;   
				}

				if(_input == "M" || _input == "N" || _input == "O" || _input == "P" \
				||	_input == "I" || _input == "J" || _input == "K"){
					dmd.clear();
					get_seq(_input);
					select_player(player);
					RED
				}

				if(_input == "D" || _input == "H"){

					if (_input == "D"){//start
						timer = start;
						status = busy;
					}
					else if(_input == "H"){ //reset
						duration = 0;
						countdown = 0;
						start_duration = 10;
					}
				}

				_input = 0;
			}
		}
	}
	else if (status == busy){
		lastmillis = millis();
		while(status == busy)
		{
			dmd.refresh();
			if (Serial.available()){
				char input = char(Serial.read());
				String _input = String(input);
				_input.trim();

				if(_input == "D" || _input == "H"){

					if (_input == "D"){//start/stop
						RED
						if(timer == start)
							timer = stop;
						else if(timer == stop)
							timer = start;
					}
					else if(_input == "H" && timer == stop){ //reset
						if(player==PAB)
							player=PCD;
						else if(player==PCD)
							player=PAB;
						dmd.clear();
						status = idle;
						select_player(player);
						countdown = duration;
						start_duration = 10;
						RED
						break;
					}

				}
			}

			if ((timer == start )&& (millis()-lastmillis >= 1000)){
				dmd.clear();

				if (countdown < 1)
				{	
					timer = stop;
					status = idle;
					countdown = duration;
					start_duration = 10;
					if(player==PAB)
						player=PCD;
					else if(player==PCD)
						player=PAB;

					RED	
					select_player(player);
					break;
				}

				if (start_duration < 1){

					select_player(player);
					if (countdown <= 30)
						YELLOW
					else 
						GREEN
					countdown = countdown - 1;

				} else {
					select_player(player);
					RED
					start_duration = start_duration - 1;
				}

	 				lastmillis = millis();
			}
		}
	}
}
