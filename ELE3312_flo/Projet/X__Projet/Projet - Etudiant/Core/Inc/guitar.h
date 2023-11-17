#ifndef GUITAR_H
#define GUITAR_H

#define SINGLEPLAYER 1
#define MULTIPLAYER 2

#define MAX_LIFE 500
#define MAX_NOTE 15

#define MAX_TIME 1000
#define DISPLAY_TIME 30

#define NOTE_SPEED 5

#define COLOR_BACKGROUND 0x0000 // BLACK
#define COLOR_SELECT 0xFFFF // WHITE

#define POSX_NO -1
#define GUITAR_POSY 200
#define GUITAR_RADIUS 10

#define FREQ_B3 246.94
#define FREQ_C4 261.63 
#define FREQ_D4 293.66
#define FREQ_D4_SHARP 311.13
#define FREQ_E4 329.63 
#define FREQ_F4_SHARP 369.99
#define FREQ_G4 392.00
#define FREQ_A4 440.00
#define FREQ_B4 493.88
#define FREQ_C5 523.25
#define FREQ_D5 587.33
#define FREQ_D5_SHARP 622.25
#define FREQ_E5 659.26
#define FREQ_F5_SHARP 698.46
#define FREQ_G5 783.99

enum note_ID {
	SILENCE,          //
	NOTE_B3,          // Si	
	NOTE_C4,          // Do	
	NOTE_D4,          // Ré	
	NOTE_D4_SHARP,    // Ré# 
	NOTE_E4,          // Mi	
	NOTE_F4_SHARP,    // Fa# 
	NOTE_G4,          // So	
	NOTE_A4,          // La	
	NOTE_B4,          // Si	
	NOTE_C5,          // Do	
	NOTE_D5,          // Ré	
	NOTE_D5_SHARP,    // Ré# 
	NOTE_E5,          // Mi	
	NOTE_F5_SHARP,    // Fa# 
	NOTE_G5};         // Sol	

#define RONDE 48
#define BLANCHE 24
#define NOIRE 12
#define CROCHE 6
#define DROCHE 3
	
struct Guitar {
	int posx;
	int note;
	int keyPressed;
};

void initGame();
void random_music_init();
void display_score();
void display_guitar(int new_position_us);
void next_state();
void gameOver();
void el_condor();

#endif //GUITAR_H
