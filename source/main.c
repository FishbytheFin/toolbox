#include <citro2d.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Helpful Varibles :)
#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240

#define PLAYER_DOWN_IDLE 0
#define PLAYER_DOWN_MOVE_0 1
#define PLAYER_DOWN_MOVE_1 2
#define PLAYER_DOWN_MOVE_2 3
#define PLAYER_DOWN_MOVE_3 4

#define PLAYER_IS_UP 0
#define PLAYER_IS_RIGHT 1
#define PLAYER_IS_DOWN 2
#define PLAYER_IS_LEFT 3

#define MAX_TONGUE_FRAMES 20

#define SCREW_SPRITE_OFFSET 6

#define SCREW_IDLE_SPRITE_0 SCREW_SPRITE_OFFSET
#define SCREW_IDLE_SPRITE_1 SCREW_SPRITE_OFFSET + 1

#define SCREW_LAUNCH_SPRITE_0 SCREW_SPRITE_OFFSET + 2
#define SCREW_LAUNCH_SPRITE_1 SCREW_SPRITE_OFFSET + 3
#define SCREW_LAUNCH_SPRITE_2 SCREW_SPRITE_OFFSET + 4

#define SCREW_SIZE 16

#define SCREW_COUNT 3

#define GROUND_SPRITE_OFFSET 10

// player height: 48px
typedef struct
{
	C2D_Sprite spr;
	float x, y; // position
} Sprite;

typedef struct
{
	C2D_Sprite sprite;			  // Sprite
	float dx, dy;				  // velocity
	float x, y;					  // position
	int w, h;					  // width, height
	int facing;					  // 0, 1, 2, 3 = up, right, down, left
	int animationFrame;			  // Animation frame to use
	int frameTime;				  // Frames until next animation
	bool tongueOut;				  // true if tongue is out
	bool tongueForward;			  // True if tongue is traveling away from the player
	float tongueTimer;			  // # of frames until the tongue is put away
	float tongueX, tongueY;		  // X & Y of the tip of the tongue compared to the x, and y + 5 respictively of the player
	float maxTongueX, maxTongueY; // The max stretch X & Y of the tip of the tongue for current lick
} Player;

typedef struct
{
	C2D_Sprite sprite;	// Sprite
	int animationFrame; // Animation frame to use
	int frameTime;		// Frames until next animation
	float dx, dy;		// velocity
	float x, y;			// position
	bool isLaunching;
	int lauchFrame;
} ScrewEnemy;

static C2D_SpriteSheet spriteSheet;
static ScrewEnemy screws[SCREW_COUNT];
static Sprite groundTiles[10];
static Player player;
static int frame;

static float cameraX, cameraY; // x & y of camera's center

// Possibly useful info for later:
// rand() % SCREEN_HEIGHT
// C2D_SpriteSetRotation(&sprite->spr, C3D_Angle(rand() / (float)RAND_MAX));

// Helper functions
static float clamp(float n, float min, float max)
{
	if (n > max)
	{
		return max;
	}
	else if (n < min)
	{
		return min;
	}
	else
	{
		return n;
	}
}

static float getCameraXOffset()
{
	return -cameraX + (SCREEN_WIDTH / 2);
}

static float getCameraYOffset()
{
	return -cameraY + (SCREEN_HEIGHT / 2);
}

// Game loop functions

static void initPlayer()
{
	Player *p = &player;

	C2D_SpriteFromSheet(&p->sprite, spriteSheet, PLAYER_DOWN_IDLE);
	C2D_SpriteSetCenter(&p->sprite, 0.5f, 0.5f);
	C2D_SpriteSetPos(&p->sprite, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

	p->dx = 0.0f;
	p->dy = 0.0f;
	p->x = SCREEN_WIDTH / 2;
	p->y = SCREEN_HEIGHT / 2;
	p->tongueOut = false;
	p->animationFrame = PLAYER_DOWN_IDLE;
	p->frameTime = 60;
}

static void initScrews()
{
	for (size_t i = 0; i < SCREW_COUNT; i++)
	{
		ScrewEnemy *screw = &screws[i];

		C2D_SpriteFromSheet(&screw->sprite, spriteSheet, SCREW_IDLE_SPRITE_0);
		C2D_SpriteSetCenter(&screw->sprite, 0.5f, 0.5f);
		C2D_SpriteSetPos(&screw->sprite, rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT);
		// C2D_SpriteSetRotation(&screw->sprite, C3D_Angle(rand()/(float)RAND_MAX));
		screw->dx = rand() * 4.0f / RAND_MAX - 2.0f;
		screw->dy = rand() * 4.0f / RAND_MAX - 2.0f;

		screw->lauchFrame = 0;
		screw->isLaunching = false;
		screw->frameTime = 0;
	}
}

static void initGroundTiles()
{
	for (size_t i = 0; i < 10; i++)
	{
		Sprite *tile = &groundTiles[i];

		C2D_SpriteFromSheet(&tile->spr, spriteSheet, GROUND_SPRITE_OFFSET + i);
		C2D_SpriteSetCenter(&tile->spr, 0.5f, 0.5f);
		C2D_SpriteSetPos(&tile->spr, rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT);
	}
}

static void init()
{
	size_t imgCount = C2D_SpriteSheetCount(spriteSheet);
	srand(time(NULL));

	initPlayer();
	initScrews();
	initGroundTiles();
}

static void playerFrame()
{
	Player *p = &player;

	p->frameTime--;
	if (p->frameTime == 0)
	{
		if (p->facing == PLAYER_IS_DOWN)
		{
			if (p->dy != 0)
			{

				if ((p->animationFrame >= PLAYER_DOWN_MOVE_3) || (p->animationFrame < PLAYER_DOWN_MOVE_0))
				{
					C2D_SpriteFromSheet(&p->sprite, spriteSheet, PLAYER_DOWN_MOVE_0);
					p->animationFrame = PLAYER_DOWN_MOVE_0;
				}
				else
				{
					p->animationFrame++;
					C2D_SpriteFromSheet(&p->sprite, spriteSheet, p->animationFrame);
				}
			}
			else
			{
				C2D_SpriteFromSheet(&p->sprite, spriteSheet, PLAYER_DOWN_IDLE);
				p->animationFrame = PLAYER_DOWN_IDLE;
			}
		}
		p->frameTime = 12;
		C2D_SpriteSetCenter(&p->sprite, 0.5f, 0.5f);
	}

	p->y = p->y + p->dy;
	p->x = p->x + p->dx;
	cameraX = p->x;
	cameraY = p->y;

	C2D_SpriteSetPos(&p->sprite, p->x + getCameraXOffset(), p->y + getCameraYOffset());
}

static void screwFrame()
{
	for (size_t i = 0; i < SCREW_COUNT; i++)
	{
		ScrewEnemy *screw = &screws[i];
		if (!(screw->isLaunching) && (rand() % 600) == 4)
		{
			screw->isLaunching = true;
			screw->lauchFrame = 60;
			C2D_SpriteFromSheet(&screw->sprite, spriteSheet, SCREW_LAUNCH_SPRITE_0);
			screw->animationFrame = SCREW_LAUNCH_SPRITE_0;
		}

		if (screw->isLaunching)
		{
		}
		else
		{
			screw->frameTime--;
			if (screw->frameTime == 0)
			{
				if ((screw->animationFrame == SCREW_IDLE_SPRITE_1) || (screw->animationFrame == SCREW_LAUNCH_SPRITE_2))
				{
					screw->frameTime = 30;
					screw->animationFrame = SCREW_IDLE_SPRITE_0;
				}
				else
				{
					screw->animationFrame++;
				}
				C2D_SpriteFromSheet(&screw->sprite, spriteSheet, screw->animationFrame);
			}
			C2D_SpriteSetCenter(&screw->sprite, 0.5f, 0.5f);
		}
		// C2D_SpriteMove(&screw->sprite, screw->dx, screw->dy);
		C2D_SpriteSetPos(&screw->sprite, screw->x + getCameraXOffset(), screw->y + getCameraYOffset());

		screw->y = screw->y + screw->dy;
		screw->x = screw->x + screw->dx;
	}
}

static void update()
{
	playerFrame();
	screwFrame();
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

		// User input
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		u32 kHeld = hidKeysHeld();
		if ((kHeld & KEY_UP))
		{
			(&player)->dy = clamp(player.dy - 0.5, -2.0f, 2.0f);
			if (!player.tongueOut)
			{
				player.facing = PLAYER_IS_UP;
			}
		}
		else if ((kHeld & KEY_DOWN))
		{
			(&player)->dy = clamp(player.dy + 0.5, -2.0f, 2.0f);
			if (!player.tongueOut)
			{
				player.facing = PLAYER_IS_DOWN;
			}
		}
		else
		{
			(&player)->dy = 0.0f;
		}

		if ((kHeld & KEY_RIGHT))
		{
			(&player)->dx = clamp(player.dx + 0.5, -2.0f, 2.0f);
			if (!player.tongueOut)
			{
				player.facing = PLAYER_IS_RIGHT;
			}
		}
		else if ((kHeld & KEY_LEFT))
		{
			(&player)->dx = clamp(player.dx - 0.5, -2.0f, 2.0f);
			if (!player.tongueOut)
			{
				player.facing = PLAYER_IS_LEFT;
			}
		}
		else
		{
			(&player)->dx = 0.0f;
		}

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
				player.facing = PLAYER_IS_RIGHT;
			}
			if (kHeld & KEY_B)
			{
				player.tongueOut = true;
				player.tongueForward = true;
				player.tongueTimer = 0;
				player.tongueY += 70;
				player.facing = PLAYER_IS_DOWN;
			}
			if (kHeld & KEY_X)
			{
				player.tongueOut = true;
				player.tongueForward = true;
				player.tongueTimer = 0;
				player.tongueY -= 70;
				player.facing = PLAYER_IS_UP;
			}
			if (kHeld & KEY_Y)
			{
				player.tongueOut = true;
				player.tongueForward = true;
				player.tongueTimer = 0;
				player.tongueX -= 70;
				player.facing = PLAYER_IS_LEFT;
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
		// Draw Screws
		for (size_t i = 0; i < 3; i++)
		{
			C2D_DrawSprite(&screws[i].sprite);
		}

		for (size_t i = 0; i < 10; i++)
		{
			C2D_DrawSprite(&groundTiles[i].spr);
		}

		// Draw player & tongue
		if (player.tongueOut)
		{
			if (player.facing == PLAYER_IS_UP)
			{
				C2D_DrawLine(player.x + getCameraXOffset(), player.y - 3 + getCameraYOffset(), C2D_Color32(255, 80, 80, 200), player.tongueX + player.x + getCameraXOffset(), player.tongueY - 3 + player.y + getCameraYOffset(), C2D_Color32(255, 80, 80, 255), 3, 0);
				C2D_DrawSprite(&player.sprite);
			}
			else
			{
				C2D_DrawSprite(&player.sprite);
				C2D_DrawLine(player.x + getCameraXOffset(), player.y - 3 + getCameraYOffset(), C2D_Color32(255, 80, 80, 200), player.tongueX + player.x + getCameraXOffset(), player.tongueY - 3 + player.y + getCameraYOffset(), C2D_Color32(255, 80, 80, 255), 3, 0);
			}
		}
		else
		{
			C2D_DrawSprite(&player.sprite);
		}

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
