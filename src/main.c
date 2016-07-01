//this example code shows how to put some text in nametable
//it assumes that you have ASCII-encoded font in the CHR tiles $00-$3f

#include "base/neslib.h"
#include "title.h"
#include "slide0.h"
#include "slide1.h"
#include "slide2.h"
#include "slide3.h"
#include "slide4.h"
#include "attributes_demo.h"


#define NTADR(x,y) ((0x2000|((y)<<5)|x))
#define PPU_ATTRIB_TABLE_0  0x23c0 // attribute table for nametable 0
#define PPU_ATTRIB_TABLE_1  0x27c0 // attribute table for nametable 1
#define PPU_ATTRIB_TABLE_2  0x2bc0 // attribute table for nametable 2
#define PPU_ATTRIB_TABLE_3  0x2fc0 // attribute table for nametable 3
#define STRING_OFFSET  0xa0
#define MAXSLIDENR 6 // start counting at 0

#pragma bss-name(push, "ZEROPAGE");
int currentSlide;
int slidenr;
unsigned char* slide;
unsigned char i, index4, bg_bright,spr_bright, sprid, frameNr;
static unsigned char other_slide;

static unsigned char joy;

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
static unsigned char playerState, playerState4, playerFast, jump, jumpStart;

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


const unsigned char* const slides[MAXSLIDENR] = {
	slide0,
	slide1,
	slide2,
	slide3,
	slide4,
	attributes_demo
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
	bank_bg(1);
	vram_adr(NAMETABLE_A);
	vram_unrle(slides[slidenr]);
	other_slide = FALSE;
	ppu_on_all();
}

void drawSprites () {
	index4 = 0;
	// buttonState4 = buttonState << 2;
	playerState4 = playerState << 2;
	for (i = 0; i < 4; ++i) {
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
	screen_state++;
	if (screen_state > 1) {
		screen_state = 0;
	}
	fade_screen_in();
	ppu_off();
	pal_bg(palBG);
	pal_spr(palSprites);
	pal_col(2,palBG[6]); // flickr mental
	drawSprites();
	render_slide();
	ppu_on_all();

}

void input (void) {

	joy = pad_trigger(0);

	playerFast = 0; // reset
	playerSpeed = 0;

	if (joy & PAD_START && screen_state == 0) {
		fade_screen_out();
		start_next_screen();
	}

	if (joy & PAD_A) {
		if (slidenr > 0) {
			--slidenr;
			other_slide = TRUE;
		}

	}
	if (joy & PAD_B) {
		if (slidenr < MAXSLIDENR) {
			++slidenr;
			other_slide = TRUE;
		}
	}

	if (joy & PAD_LEFT) {
		playerSpeed = -1;
	}
	if (joy & PAD_RIGHT) {
		playerSpeed = 1;
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
	input();
	drawSprites();
	playerMovement();
}

unsigned char pal_item = 0;
void main(void) {
	other_slide = FALSE;
	frameNr = 0;
	start_title_screen();
	ppu_on_all();

	slidenr = 0;

	pal_col(0,0x0d);

	while(1) {
		ppu_wait_nmi();
		if (screen_state == 0) {
			if (pal_item < 15) {
				pal_item = pal_item + 1;
			} else{
				pal_item = 0;
			}
			input();
			pal_col(2,palSprites[pal_item]); // flickr mental
		} else {
			render_slide();
			while(!other_slide) {
				game_logic();
			}
		}
	}
}
