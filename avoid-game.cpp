#include <LiquidCrystal.h>
#include <Arduino.h>


#define SPRITE_SHIP 1            //use the character ship
#define SPRITE_METEOR 2         //'' '' '' meteor
#define SPRITE_METEOR_EMPTY ' '  // Use the ' ' character

#define TERRAIN_WIDTH 16         //width of the screen

#define SHIP_HORIZONTAL_POSITION 1    // Horizontal position of hero on screen

//state of the meteor
#define METEOR_EMPTY 0
#define METEOR_LOWER_BLOCK 1
#define METEOR_UPPER_BLOCK 2

//state of the shot
#define SHOT_EMPTY 0
#define SHOT_LOWER_BLOCK 1
#define SHOT_UPPER_BLOCK 2

//state of the ship
#define SHIP_POSITION_OFF 0
#define SHIP_POSITION_LOWER 1
#define SHIP_POSITION_UPPER 2


//declaration of the pins for the screen
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//define one each for the line
char terrainUpper[TERRAIN_WIDTH + 1];
char terrainLower[TERRAIN_WIDTH + 1];

//function used for the time of loop that reduce for each loop
unsigned long lastGameUpdateTick = 0;
unsigned long gameUpdateInterval = 1000;

// creation of the new character  put in ccgram
void initializeGraphics(){
	byte graphics[] = {
			//ship
			B11000,
			B01100,
			B00110,
			B11111,
			B11111,
			B00110,
			B01100,
			B11000,
			//meteor
			B00000,
			B00000,
			B00110,
			B01111,
			B01111,
			B00110,
			B00000,
			B00000,
	};
	int i;

    // Skip using character 0, this allows lcd.print() to be used to
    // quickly draw multiple characters
    for (i = 0; i < 2; ++i) {
        lcd.createChar(i + 1, &graphics[i * 8]);
    }
    //initialize the screen empty
    for (i = 0; i < TERRAIN_WIDTH; ++i) {
            terrainUpper[i] = SPRITE_METEOR_EMPTY;
            terrainLower[i] = SPRITE_METEOR_EMPTY;
        }
    gameUpdateInterval = 100;
}

 //move the meteor
void advanceMeteor(char* meteor, byte newMeteor){
	for (int i =0; i < TERRAIN_WIDTH; ++i){
		char current = meteor[i];
		char next = (i == TERRAIN_WIDTH-1) ? newMeteor : meteor[i+1];

		// depend of the status of the meteor
		switch (current){
			case SPRITE_METEOR_EMPTY: // if empty and next one full put in this case full else empty
				meteor[i] =(next == SPRITE_METEOR) ? SPRITE_METEOR : SPRITE_METEOR_EMPTY;
			break;

			case SPRITE_METEOR://if full next one is empty put empty else full
				meteor[i] = (next == SPRITE_METEOR_EMPTY) ? SPRITE_METEOR_EMPTY : SPRITE_METEOR;
			break;
		}
	}
}


// to move the ship upper or lower  and look if there is a collision
boolean drawShip(byte position, char* terrainUpper, char* terrainLower, unsigned int score ){
	boolean collide = false;
	char upperSave = terrainUpper[SHIP_HORIZONTAL_POSITION];
	char lowerSave = terrainLower[SHIP_HORIZONTAL_POSITION];
	byte upper,lower;

	switch (position){ //if status empty don't print the screen
		case SHIP_POSITION_OFF:
			upper = lower = SPRITE_METEOR_EMPTY;
			break;
		case SHIP_POSITION_LOWER: //if position is lower print the ship at the bottom
			upper = SPRITE_METEOR_EMPTY;
			lower = SPRITE_SHIP;
			break;
		case SHIP_POSITION_UPPER: //if position is upper print the ship at the top
			upper = SPRITE_SHIP;
			lower = SPRITE_METEOR_EMPTY;
			break;
	}

		if (upper != ' ') {//look if there is a collision at the upper line
	        terrainUpper[SHIP_HORIZONTAL_POSITION] = upper;
	        collide = (upperSave == SPRITE_METEOR_EMPTY) ? false : true;
	    }

	    if (lower != ' ') { //look if there is a collision at the lower line
	        terrainLower[SHIP_HORIZONTAL_POSITION] = lower;
	        collide |= (lowerSave == SPRITE_METEOR_EMPTY) ? false : true;
	    }

	    byte digits = (score > 9999) ? 5 : (score > 999) ? 4 : (score > 99) ? 3 : (score > 9) ? 2 : 1;

	    // Draw the scene
	    terrainUpper[TERRAIN_WIDTH] = '\0';
	    terrainLower[TERRAIN_WIDTH] = '\0';
	    char temp = terrainUpper[16-digits];
	    terrainUpper[16-digits] = '\0';
	    lcd.setCursor(0,0);
	    lcd.print(terrainUpper);
	    terrainUpper[16-digits] = temp;
	    lcd.setCursor(0,1);
	    lcd.print(terrainLower);

	    lcd.setCursor(16 - digits,0);
	    lcd.print(score);

	    terrainUpper[SHIP_HORIZONTAL_POSITION] = upperSave;
	    terrainLower[SHIP_HORIZONTAL_POSITION] = lowerSave;


	    return collide;
}

void setup (){

	// BUTTONS CONTROLLER
	DDRD &= ~(1<<DDD0) | ~(1<<DDD1) | ~(1<<DDD2) | ~(1<<DDD3); // Direction of port line (0 - input)

	//pinmode

    initializeGraphics();

    lcd.begin(16, 2);
}

void loop(){
    static byte shipPos = SHIP_POSITION_LOWER;
    static byte newMeteorType = METEOR_EMPTY;
    static byte newShotType = SHOT_EMPTY;
    static byte newMeteorDuration = 1;
    static boolean playing = false;
    static boolean blink = false;
    static unsigned int distance = 0;
    int counter = 0;
    int length = 5;
    int i,y = 0;
    int rand ;

    //if not playing blink the sreen and print "press start" and if one button press set the position of the ship lower and start the game
    if (!playing){
    	drawShip((blink) ? SHIP_POSITION_OFF : shipPos, terrainUpper, terrainLower, distance >> 3);

    	if (blink) {
    	lcd.setCursor(0,0);
    	lcd.print("Press Start");
    	}

    	delay(250);
    	blink = !blink;

    	if (!(PIND & 1<<PIND1) || !(PIND & 1<<PIND0)){
    		initializeGraphics();
    		shipPos = SHIP_POSITION_LOWER;
    		playing = true;
    		distance = 0;
    	}
    	return;
    }

    // Shift the terrain to the left
    advanceMeteor(terrainLower, newMeteorType == METEOR_LOWER_BLOCK ? SPRITE_METEOR : SPRITE_METEOR_EMPTY);
    advanceMeteor(terrainUpper, newMeteorType == METEOR_UPPER_BLOCK ? SPRITE_METEOR : SPRITE_METEOR_EMPTY);


    //try
    	if (rand == 0){
    		i ++;
    	 }else {
    		 y++;
    	 }
    	if (y == 3){
    		rand = 0;
    		y = 0;
    		i = 0;
    	}else if(i == 3){
    		rand = random(1,2);
    		y = 0;
    		i = 0;
    	}


    // Make new terrain to enter on the right
        if (--newMeteorDuration == 0) {
            if (newMeteorType == METEOR_EMPTY) {
            	rand = random(2);
            	newMeteorType = (rand == 0) ? METEOR_UPPER_BLOCK : METEOR_LOWER_BLOCK;
                newMeteorDuration = 2 + random(3);
            } else {
            	newMeteorType = METEOR_EMPTY;
                newMeteorDuration = 2 + random(length);
            }
        }

        if (!(PIND & 1<<PIND1)){
        	if (shipPos == SHIP_POSITION_UPPER){
        		shipPos = SHIP_POSITION_LOWER;
        	}
        }
        if (!(PIND & 1<<PIND0)){
            if (shipPos == SHIP_POSITION_LOWER){
              	shipPos = SHIP_POSITION_UPPER;
            }
        }

        if (drawShip(shipPos, terrainUpper, terrainLower,distance >> 3)){
        	playing = false; // The hero collided with something. Too bad.
        }else {
        	distance ++;
        }


        if(millis()-lastGameUpdateTick>gameUpdateInterval){
            lastGameUpdateTick = millis();
            counter ++;
            if (counter == 10 && length >1){
            	length --;
            	counter = 0;
            	gameUpdateInterval = gameUpdateInterval-5;

            }
          }

        delay(gameUpdateInterval);

}
