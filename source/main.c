#include <citro2d.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Helpful Varibles :)
#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240

#define TEMP_PLAYER_SPRITE 0

#define PLAYER_IS_UP 0
#define PLAYER_IS_RIGHT 1
#define PLAYER_IS_DOWN 2
#define PLAYER_IS_LEFT 3

#define MAX_TONGUE_FRAMES 20

// player height: 48px
typedef struct
{
	C2D_Sprite spr;
	float dx, dy; // velocity
} Sprite;

typedef struct
{
	C2D_Sprite sprite;			  // Sprite
	float dx, dy;				  // velocity
	int x, y;					  // position
	int w, h;					  // width, height
	int facing;					  // 0, 1, 2, 3 = up, right, down, left
	bool tongueOut;				  // true if tongue is out
	bool tongueForward;			  // True if tongue is traveling away from the player
	float tongueTimer;			  // # of frames until the tongue is put away
	float tongueX, tongueY;		  // X & Y of the tip of the tongue compared to the x, and y + 5 respictively of the player
	float maxTongueX, maxTongueY; // The max stretch X & Y of the tip of the tongue for current lick
} Player;

static C2D_SpriteSheet spriteSheet;
static Sprite sprites[4];
static Player player;
static int frame;

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
	p->tongueOut = false;
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

	frame = 0;

	// Sandwhich
	init();

	printf("Hello, world!\n");
	printf("I am sandwhich\n");

	// Main loop
	while (aptMainLoop())
	{

		frame++;
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
		if (!player.tongueOut)
		{
			player.tongueX = 0;
			player.tongueY = 0;
			if (kHeld & KEY_A)
			{
				player.tongueOut = true;
				player.tongueForward = true;
				player.tongueTimer = 0;
				player.tongueX += 70;
			}
			if (kHeld & KEY_B)
			{
				player.tongueOut = true;
				player.tongueForward = true;
				player.tongueTimer = 0;
				player.tongueY += 70;
			}
			if (kHeld & KEY_X)
			{
				player.tongueOut = true;
				player.tongueForward = true;
				player.tongueTimer = 0;
				player.tongueY -= 70;
			}
			if (kHeld & KEY_Y)
			{
				player.tongueOut = true;
				player.tongueForward = true;
				player.tongueTimer = 0;
				player.tongueX -= 70;
			}

			if (player.tongueX == 0)
			{
				// player.tongueY += (player.tongueY - (player.y + 5)) * 2;
				player.tongueY *= 1.5;
			}
			else if (player.tongueY == 0)
			{
				// player.tongueX += (player.tongueX - player.x) * 2;
				player.tongueX *= 1.5;
			}

			if (player.tongueOut)
			{
				player.maxTongueX = player.tongueX;
				player.maxTongueY = player.tongueY;
				player.tongueX = 0;
				player.tongueY = 0;
			}
		}
		else
		{

			if (player.tongueForward)
			{
				player.tongueTimer++;
			}
			else
			{
				player.tongueTimer--;
			}
			player.tongueX = player.maxTongueX * (player.tongueTimer / MAX_TONGUE_FRAMES);
			player.tongueY = player.maxTongueY * (player.tongueTimer / MAX_TONGUE_FRAMES);

			if (player.tongueTimer == MAX_TONGUE_FRAMES)
			{
				player.tongueForward = false;
			}
			else if (player.tongueTimer == 0)
			{
				player.tongueOut = false;
			}
		}

		update();

		// Render
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
		C2D_SceneBegin(top);

		// Draw sprites
		C2D_DrawSprite(&player.sprite);
		if (player.tongueOut)
		{
			C2D_DrawLine(player.x, player.y + 5, C2D_Color32(255, 80, 80, 200), player.tongueX + player.x, player.tongueY + 5 + player.y, C2D_Color32(255, 80, 80, 255), 3, 0);
		}

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
