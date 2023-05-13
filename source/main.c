#include <citro2d.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240

#define TEMP_PLAYER_SPRITE 0

typedef struct
{
	C2D_Sprite spr;
	float dx, dy; // velocity
} Sprite;

typedef struct
{
	C2D_Sprite sprite; // Sprite
	float dx, dy;	   // velocity
	int x, y;		   // position
	int w, h;		   // width, height
} Player;

static C2D_SpriteSheet spriteSheet;
static Sprite sprites[4];
static Player player;

// Possibly useful info for later:
// rand() % SCREEN_HEIGHT
// C2D_SpriteSetRotation(&sprite->spr, C3D_Angle(rand() / (float)RAND_MAX));

static void init()
{
	size_t imgCount = C2D_SpriteSheetCount(spriteSheet);
	srand(time(NULL));

	Player *p = &player;

	C2D_SpriteFromSheet(&p->sprite, spriteSheet, TEMP_PLAYER_SPRITE);
	C2D_SpriteSetCenter(&p->sprite, 0.5f, 0.5f);
	C2D_SpriteSetPos(&p->sprite, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

	p->dx = 0.0f;
	p->dy = 0.0f;
	p->x = SCREEN_WIDTH / 2;
	p->y = SCREEN_HEIGHT / 2;
}

static void movePlayer()
{
	Player *p = &player;
	C2D_SpriteMove(&p->sprite, p->dx, p->dy);

	p->y = p->y + p->dy;
	p->x = p->x + p->dx;
}

static void update()
{
	movePlayer();
}

static void moveBoys()
{
	for (size_t i = 0; i < 4; i++)
	{
		Sprite *sprite = &sprites[i];

		C2D_SpriteMove(&sprite->spr, sprite->dx, sprite->dy);
		C2D_SpriteRotateDegrees(&sprite->spr, 1.0f);

		// Check for collision with the screen boundaries
		if ((sprite->spr.params.pos.x < sprite->spr.params.pos.w / 2.0f && sprite->dx < 0.0f) ||
			(sprite->spr.params.pos.x > (SCREEN_WIDTH - (sprite->spr.params.pos.w / 2.0f)) && sprite->dx > 0.0f))
			sprite->dx = -sprite->dx;

		if ((sprite->spr.params.pos.y < sprite->spr.params.pos.h / 2.0f && sprite->dy < 0.0f) ||
			(sprite->spr.params.pos.y > (SCREEN_HEIGHT - (sprite->spr.params.pos.h / 2.0f)) && sprite->dy > 0.0f))
			sprite->dy = -sprite->dy;
	}
}

int main(int argc, char *argv[])
{
	// Init
	romfsInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	consoleInit(GFX_BOTTOM, NULL); // Console on lower screen

	// Create screens
	C3D_RenderTarget *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

	spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
	if (!spriteSheet)
		svcBreak(USERBREAK_PANIC);

	// Sandwhich
	init();

	printf("Hello, world!\n");
	printf("I am sandwhich\n");

	// Main loop
	while (aptMainLoop())
	{
		hidScanInput();

		// Your code goes here
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		u32 kHeld = hidKeysHeld();
		if ((kHeld & KEY_UP))
			(&player)->dy = -2.0f;
		else if ((kHeld & KEY_DOWN))
			(&player)->dy = 2.0f;
		else
			(&player)->dy = 0.0f;

		if ((kHeld & KEY_RIGHT))
			(&player)->dx = 2.0f;
		else if ((kHeld & KEY_LEFT))
			(&player)->dx = -2.0f;
		else
			(&player)->dx = 0.0f;

		update();

		// Render
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
		C2D_SceneBegin(top);

		// Draw sprites
		C2D_DrawSprite(&player.sprite);

		// C2D_DrawRectangle(0.0f, 0.0f, 0.0f, 20.0f, 90.0f, 255.0f, 0.0f, 0.0f, 1.0f);
		C3D_FrameEnd(0);
	}

	// Delete graphics
	C2D_SpriteSheetFree(spriteSheet);

	C2D_Fini();
	C3D_Fini();
	gfxExit();
	romfsExit();
	return 0;
}
