//this example code shows how to put some text in nametable
//it assumes that you have ASCII-encoded font in the CHR tiles $00-$3f

#include "base/neslib.h"
#include "title.h"
#include "slide_attributes.h"


#define NTADR(x,y) ((0x2000|((y)<<5)|x))
#define PPU_ATTRIB_TABLE_0  0x23c0 // attribute table for nametable 0
#define PPU_ATTRIB_TABLE_1  0x27c0 // attribute table for nametable 1
#define PPU_ATTRIB_TABLE_2  0x2bc0 // attribute table for nametable 2
#define PPU_ATTRIB_TABLE_3  0x2fc0 // attribute table for nametable 3
#define STRING_OFFSET  0xa0
#define MAXSLIDENR 3

#pragma bss-name(push, "ZEROPAGE");
unsigned char currentSlide = 0;
unsigned char i, index4, bg_bright,spr_bright, sprid, frameNr;

static unsigned char joy;
static unsigned char joytrig;
static unsigned char buttonX = 20;
static unsigned char buttonY = 200;
unsigned char sprButtonX[]={ 0,	8,	0,	8 };
unsigned char sprButtonY[] = { -1, -1, 7, 7 };
unsigned char sprButtonTile[] = {
	0x04,0x05,0x14,0x15, // up
	0x06,0x07,0x16,0x17  // down
};
unsigned char sprButtonAttr[] = { 3, 3, 3, 3 };

unsigned char sprPlayerX[]={ 0,	8,	0,	8 };
unsigned char sprPlayerY[] = { -1, -1, 7, 7 };
unsigned char sprPlayerTile[] = {
	0x08,0x09,0x18,0x19, // normal
	0x0A,0x0B,0x1A,0x1B,  // not normal
	0x08,0x09,0x18,0x19  // jump
};
unsigned char sprPlayerAttr[] = { 0, 0, 0, 0 };
unsigned char playerX = 60;
unsigned char playerY = 200;
unsigned char playerOriginY = 200;
unsigned char playerSpeed = 0;
unsigned char playerHotSpots[] = {
	8, 0, // top
	0, 8, // left
	8, 16, // bottom
	16, 8 // right
};

unsigned char buttonHotspots[] = {
	8, 0, // top
	0, 8, // left
	8, 16, // bottom
	16, 8 // right
};

unsigned char collisionState = 0;
static unsigned char buttonState, buttonState4;
static unsigned char playerState, playerState4, playerFast, jump, jumpStart;
int slidenr;

const unsigned char* screens[] = {
	title,
	slide_attributes
};

unsigned char screen_state; // 0 == title 1 == game 2 == bla

#pragma bss-name(push, "OAM")
unsigned char SPRITES[256];


const unsigned char palSprites[16]={
	0x33,0x15,0x25,0x35,
	0x33,0x19,0x29,0x39,
	0x33,0x11,0x21,0x31,
	0x33,0x17,0x27,0x07
};

const unsigned char palBG[16]={
	0x33,0x15,0x22,0x20,
	0x33,0x01,0x21,0x31,
	0x33,0x15,0x25,0x36,
	0x33,0x2c,0x13,0x04
};


const unsigned char* slide_1[3] = {
	"VAR NESLY = REQUIRE('NESLY')",
	"HENK",
	"YES THREE LINES",
};



void put_str(unsigned int adr,const char *str)
{
	vram_adr(adr);
	while(1)
	{
		if(!*str) break;

		vram_put((*str++) + STRING_OFFSET);//-0x20 because ASCII code 0x20 is placed in tile 0 of the CHR
	}
}


void fade_screen_in(void)
{
	for(i=0;i<16;i++)
	{
		if(!(i&3))
		{
			bg_bright++;
			pal_bg_bright(bg_bright);
			spr_bright++;
			pal_spr_bright(spr_bright);
		}
	}
}

void fade_screen_out(void)
{
	for(i=0;i<16;i++)
	{
		ppu_wait_nmi();
		if(!(i&3)) {
			bg_bright--;
			pal_bg_bright(bg_bright);
			spr_bright--;
			pal_spr_bright(spr_bright);
		}
	}
}

void render_slide () {
	ppu_off();
	for (i = 0; i < 3; i++) {
		put_str(NTADR_A(2, 2 * (int)i + 2), slide_1[(int)i]);
	}
	ppu_on_all();
}

void drawSprites () {
	index4 = 0;
	buttonState4 = buttonState << 2;
	playerState4 = playerState << 2;
	for (i = 0; i < 4; ++i) {
		SPRITES[index4] = sprButtonY[i] + buttonY;
		++index4;
		SPRITES[index4] = sprButtonTile[i + buttonState4];
		++index4;
		SPRITES[index4] = sprButtonAttr[i];
		++index4;
		SPRITES[index4] = sprButtonX[i] + buttonX;
		++index4;
		SPRITES[index4] = sprPlayerY[i] + playerY;
		++index4;
		SPRITES[index4] = sprPlayerTile[i + playerState4];
		++index4;
		SPRITES[index4] = sprPlayerAttr[i];
		++index4;
		SPRITES[index4] = sprPlayerX[i] + playerX;
		++index4;
	}
}

void start_next_screen () {
	buttonState = 0;
	screen_state++;
	if (screen_state > 1) {
		screen_state = 0;
	}
	fade_screen_in();
	ppu_off();
	vram_adr(NAMETABLE_A);
	vram_unrle(screens[screen_state]);
	render_slide();
	pal_bg(palBG);
	pal_spr(palSprites);
	drawSprites();
	ppu_on_all();
}

void input (void) {
	joytrig=pad_trigger(0);
	joy=pad_poll(0);

	playerFast = 0; // reset
	playerSpeed = 0;

	if (joy & PAD_START && screen_state == 0) {
		fade_screen_out();
		start_next_screen();
		slidenr = 0;
	}

	if (joy & PAD_A) {
		if (jump != 1) {
			jump = 1;
			jumpStart = frameNr;
		}
	}
	if (joy & PAD_B) {
		playerFast = 1;
	}

	if (joy & PAD_LEFT) {
		playerSpeed = -1;
		if (playerFast == 1) {
			playerSpeed = -2;
		}
	}
	if (joy & PAD_RIGHT) {
		playerSpeed = 1;
		if (playerFast == 1) {
			playerSpeed = 2;
		}
	}
}

void start_title_screen () {
	fade_screen_in();
	ppu_off();
	pal_bg(palBG);
	bank_bg(1);
	vram_adr(NAMETABLE_A);
	vram_unrle(title);
	put_str(NTADR_A(10, 22), "PRESS START");
	ppu_on_all();
}

void collisions () {
	collisionState = 0;
	for (i = 0; i < 4; i++) {
		if (playerX + playerHotSpots[i] == buttonHotspots[i] + buttonX &&
			  playerY + playerHotSpots[i+1] == buttonHotspots[i+1]) {
			collisionState = 1;
		}
	}
}

void playerMovement () {
	if (jump == 1 && frameNr > jumpStart + 120) {
		--playerY;
	}

	if (frameNr > jumpStart + 120) {
		jump = 0;
	}

	if (playerY > playerOriginY) {
		++playerY;
	}

	if (collisionState != 1) {
		playerX = playerX + playerSpeed;
	}

}


void game_logic () {
	collisions();
	input();
	drawSprites();
	playerMovement();
	putstr(NTADR_A(20, 20), playerX)
}

unsigned char pal_item = 0;
void main(void) {
	frameNr = 0;
	start_title_screen();
	ppu_on_all();

	pal_col(0,0x0d);

	while(1) {
		ppu_wait_nmi();
		++frameNr;
		if (screen_state == 0) {
			if (pal_item < 15) {
				pal_item = pal_item + 1;
			} else{
				pal_item = 0;
			}
			input();
			pal_col(2,palSprites[pal_item]); // flickr mental
		} else if (screen_state == 1) {
			game_logic();
		}
	}
}
