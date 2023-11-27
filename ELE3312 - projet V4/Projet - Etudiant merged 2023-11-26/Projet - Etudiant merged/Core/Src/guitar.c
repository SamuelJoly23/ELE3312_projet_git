#include "guitar.h"
#include "ili9341.h"
#include "ili9341_gfx.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "main.h"
#include "tim.h"

ili9341_t *_screen;

extern int full_timer;
extern int life;
extern UART_HandleTypeDef huart5;


volatile int nextNoteUp;
volatile float position_us = 1000;

int POSX[MAX_NOTE+1] = {0};	// Positions horizontale des centres des notes sur le LCD
float FREQX[MAX_NOTE+1] = {0, FREQ_B3, FREQ_C4, FREQ_D4, FREQ_D4_SHARP, FREQ_E4, FREQ_F4_SHARP, FREQ_G4, FREQ_A4, 
                              FREQ_B4, FREQ_C5, FREQ_D5, FREQ_D5_SHARP, FREQ_E5, FREQ_F5_SHARP, FREQ_G5};	//Fréquences des notes

struct Guitar guitar;
struct Guitar guitar2;															
unsigned char voice1[MAX_TIME] = {0};	// Tableau temporel des notes pour la premiere voix
unsigned char voice2[MAX_TIME] = {0};	// Tableau temporel des notes pour la seconde voix

int current_time = 0;

void initGame(){
	ili9341_fill_screen(_screen, COLOR_BACKGROUND);
	
	for (int i=1; i<=MAX_NOTE; i++) {
		POSX[i] = (320*i-160)/MAX_NOTE;
	}

	guitar.posx = POSX_NO;
	nextNoteUp = 0;
	
	life = MAX_LIFE;
	full_timer = 0;
	current_time = 0;
	
	display_score();
	display_guitar(position_us);
	el_condor();
}

void random_music_init() {
	voice1[0] = rand()%(MAX_NOTE+1);
	voice2[0] = rand()%(MAX_NOTE+1);
	
	for (int i=1; i<MAX_TIME; i++) {
		int new_voice1 = voice1[i-1];
		if ((rand()%5) == 0) {
			new_voice1 += (rand()%5)-2;
			if (new_voice1<0) new_voice1 = 0;
			if (new_voice1>MAX_NOTE) new_voice1 = MAX_NOTE;
		}
		voice1[i] = new_voice1;
		
		int new_voice2 = voice2[i-1];
		if ((rand()%5) == 0) {
		new_voice2 += (rand()%5)-2;
			if (new_voice2<0) new_voice2 = 0;
			if (new_voice2>MAX_NOTE) new_voice2 = MAX_NOTE;
		}
		voice2[i] = new_voice2;
	}
}

void display_score() {
	int local_time = current_time;
	int current_y = GUITAR_POSY;
	const int delta_y = GUITAR_POSY / DISPLAY_TIME;
		
	for (int i = 0; i<DISPLAY_TIME; i++) {
		for (int j=1; j<=MAX_NOTE; j++) {
			ili9341_fill_rect(_screen, COLOR_BACKGROUND, POSX[j]-6, current_y, 12, delta_y);
			if (voice1[local_time] == j) {
				ili9341_fill_rect(_screen, ILI9341_GREEN, POSX[j]-6, current_y, 6, delta_y);
			} 
			if (voice2[local_time] == j) {
				ili9341_fill_rect(_screen, ILI9341_BLUE, POSX[j]-0, current_y, 6, delta_y);
			} 
		}
		local_time++;
		if (local_time >= MAX_TIME) local_time = 0;
		current_y -= delta_y;
	}
}
/*
void display_guitar(int new_position_us) {
	//Erase old chord selected
	if(guitar.posx != POSX_NO){
		ili9341_draw_circle(_screen, COLOR_BACKGROUND, guitar.posx, GUITAR_POSY, GUITAR_RADIUS+1);
	}
	
	const int max_us = 1750;
	if (new_position_us >= max_us){
		guitar.note = 0;
		guitar.posx = POSX_NO;
	} else {
		guitar.note = 1 + (new_position_us * MAX_NOTE) / max_us ;
		guitar.posx = POSX[guitar.note];
	}
	
	//Draw new chord selected
	if(guitar.posx != POSX_NO){
		ili9341_draw_circle(_screen, ILI9341_GREEN, guitar.posx, GUITAR_POSY, GUITAR_RADIUS+1);
	}
}
*/

// new version of display_guitar using ID_note instead of position
void display_guitar(int note) {
	//Erase old chord selected
	if(guitar.posx != POSX_NO){
		ili9341_draw_circle(_screen, COLOR_BACKGROUND, guitar.posx, GUITAR_POSY, GUITAR_RADIUS+1);
	}
	
	guitar.note = note;
	
	if(guitar.note == 0){
		guitar.posx = POSX[1];
	}
	else(guitar.posx = POSX[guitar.note]);
	
	
	//Draw new chord selected
	if(guitar.posx != POSX_NO){
		ili9341_draw_circle(_screen, ILI9341_GREEN, guitar.posx, GUITAR_POSY, GUITAR_RADIUS+1);
	}
}


/////////////////////////////////////////////////////////////////////////////////////
void display_guitar2(int note2) {
	//Erase old chord selected
	
	if(guitar2.posx != POSX_NO){
		ili9341_draw_circle(_screen, COLOR_BACKGROUND, guitar2.posx, GUITAR_POSY, GUITAR_RADIUS+1);
	}
	guitar2.note = note2;
	
	if(guitar2.note == 0){
		guitar2.posx = POSX[1];
	}
	else(guitar2.posx = POSX[guitar2.note]);
	
	
	//Draw new chord selected
	if(guitar2.posx != POSX_NO){
		ili9341_draw_circle(_screen, ILI9341_BLUE, guitar2.posx, GUITAR_POSY, GUITAR_RADIUS+1);
	}
}
///////////////////////////////////////////////////////////////////////////////////////
void next_state() {
	if (guitar.note != voice1[current_time]) life--;
	current_time++;
	if (current_time == MAX_TIME) current_time = 0;

//DEBUG AUTOPLAY
	int posx = (320*voice1[current_time]-160)/MAX_NOTE;
	position_us = 1750 * posx/320;
//END DEBUG AUTOPLAY
}

void gameOver(){
	ili9341_fill_screen(_screen, COLOR_BACKGROUND);
	char buffer[20] = {0};
	sprintf(buffer, "GAME OVER");
	ili9341_text_attr_t text_attr = {&ili9341_font_16x26,ILI9341_RED, COLOR_BACKGROUND,75,90};
	ili9341_draw_string(_screen, text_attr,buffer);
	
	char buffer1[20] = {0};
	int score = life*1000; 
	sprintf(buffer1, "Score : %i",score);
	ili9341_text_attr_t text_attr1 = {&ili9341_font_11x18,ILI9341_WHITE, COLOR_BACKGROUND,82,135};
	ili9341_draw_string(_screen, text_attr1,buffer1);
	
	HAL_Delay(5000);
}
void liveScore(){
	char buffer1[20] = {0};
	uint32_t score = (double)full_timer*(double)life/60.0;
	sprintf(buffer1, "Score : %u",score);
	ili9341_text_attr_t text_attr1 = {&ili9341_font_11x18,ILI9341_WHITE, COLOR_BACKGROUND,160,0};
	ili9341_draw_string(_screen, text_attr1,buffer1);
}

void add_notes(unsigned char * voice, int nb_notes, unsigned char note_array[], int length_array[]) {
	int current_time = 0;
	
	for (int i=0; i<nb_notes; i++) {
		int current_length = length_array[i]-1;
		while (current_length) {
			voice[current_time++] = note_array[i];
			current_length--;
			if (current_time == MAX_TIME) return;
		}
		voice[current_time++] = 0;
		if (current_time == MAX_TIME) return;
	}
}

unsigned char el_condor_notes[100] = {//joueur 1
	NOTE_B3, NOTE_E4, NOTE_D4_SHARP, NOTE_E4, NOTE_F4_SHARP, NOTE_G4, NOTE_F4_SHARP, NOTE_G4, NOTE_A4, 
	NOTE_B4,
	SILENCE, NOTE_D5, 
	NOTE_B4,
	SILENCE, NOTE_E5, NOTE_D5, 
	NOTE_B4,
	SILENCE, NOTE_A4, NOTE_G4, 
	NOTE_E4,
	SILENCE, NOTE_G4,
	NOTE_E4
};

int el_condor_length[100] = {
	NOIRE, NOIRE, NOIRE, NOIRE, NOIRE, NOIRE, NOIRE, NOIRE, NOIRE, 
	RONDE,
	BLANCHE, BLANCHE, 
	RONDE,
	BLANCHE, NOIRE, NOIRE, 
	RONDE,
	BLANCHE, NOIRE, NOIRE, 
	RONDE,
	BLANCHE, BLANCHE,
	RONDE
};

unsigned char el_condor_notes_bass[100] = {
	SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, SILENCE, 
	NOTE_G4, NOTE_D4,
	NOTE_G4, NOTE_D4,
	NOTE_G4, NOTE_D4,
	NOTE_G4, NOTE_D4,
	NOTE_G4, NOTE_D4,
	NOTE_G4, NOTE_D4,
	NOTE_E4, NOTE_B3,
	NOTE_E4, NOTE_B3,
	NOTE_E4, NOTE_B3
};

int el_condor_length_bass[100] = {
	NOIRE, NOIRE, NOIRE, NOIRE, NOIRE, NOIRE, NOIRE, NOIRE, NOIRE, 
	BLANCHE,BLANCHE,
	BLANCHE,BLANCHE,
	BLANCHE,BLANCHE,
	BLANCHE,BLANCHE,
	BLANCHE,BLANCHE,
	BLANCHE,BLANCHE,
	BLANCHE,BLANCHE,
	BLANCHE,BLANCHE,
	BLANCHE,BLANCHE
};

void el_condor() {
	if(PLAYER == 1){
	add_notes(voice1, 25, el_condor_notes, el_condor_length);
	}
	else{
	add_notes(voice2, 27, el_condor_notes_bass, el_condor_length_bass);
	}
}

void display_received_old(unsigned char received_data){

	
	char buffer1[20] = {0};
	sprintf(buffer1, "Data received: %i",received_data);
	ili9341_text_attr_t text_attr1 = {&ili9341_font_11x18,ILI9341_WHITE, COLOR_BACKGROUND,82,135};
	ili9341_draw_string(_screen, text_attr1,buffer1);
	
}


void display_received(unsigned char received_data){	
	int local_time = current_time;
	int current_y = GUITAR_POSY;
	const int delta_y = GUITAR_POSY / DISPLAY_TIME;
		
	for (int i = 0; i<DISPLAY_TIME; i++) {
		for (int j=1; j<=MAX_NOTE; j++) {
			ili9341_fill_rect(_screen, COLOR_BACKGROUND, POSX[j]-6, current_y, 12, delta_y);
			if (voice1[local_time] == j) {
				ili9341_fill_rect(_screen, ILI9341_GREEN, POSX[j]-6, current_y, 6, delta_y);
			} 
			if (voice2[local_time] == j) {
				ili9341_fill_rect(_screen, ILI9341_BLUE, POSX[j]-0, current_y, 6, delta_y);
			} 
		}
		local_time++;
		if (local_time >= MAX_TIME) local_time = 0;
		current_y -= delta_y;
	}
	
}


void add_notes_future(unsigned char * voice, unsigned char received_data) {
	int local_time = current_time;
	//for (int i = 0; i<30; i++){
	voice[local_time] = received_data;
	guitar2.note = received_data;
	//}
	
}


void displaying_test(unsigned char note){
	char buffer1[20] = {0};
	sprintf(buffer1, "next note : %i", note );
	ili9341_text_attr_t text_attr1 = {&ili9341_font_11x18,ILI9341_WHITE, COLOR_BACKGROUND,130,135};
	ili9341_draw_string(_screen, text_attr1,buffer1);
}




void display_score_2() {
	int local_time = current_time;
	int current_y = GUITAR_POSY;
	const int delta_y = GUITAR_POSY / DISPLAY_TIME;
		
	for (int i = 0; i<DISPLAY_TIME; i++) {
		for (int j=1; j<=MAX_NOTE; j++) {
			ili9341_fill_rect(_screen, COLOR_BACKGROUND, POSX[j]-6, current_y, 12, delta_y);
//			if (voice1[local_time] == j) {
//				ili9341_fill_rect(_screen, ILI9341_GREEN, POSX[j]-6, current_y, 6, delta_y);
//			} 
			if (voice2[local_time] == j) {
				ili9341_fill_rect(_screen, ILI9341_BLUE, POSX[j]-0, current_y, 6, delta_y);
			} 
		}
		local_time++;
		if (local_time >= MAX_TIME) local_time = 0;
		current_y -= delta_y;
	}
}

void display_player2(struct Guitar guitar2){
	int posx = (320*guitar2.note-160)/MAX_NOTE;
	guitar2.posx = 1750 * posx/320;
}



