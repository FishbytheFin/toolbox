#include <citro2d.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/*
TODO:
FIX GROUND TILES
FINAL BOSS

*/

// Helpful Varibles :)
#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240

#define PLAYER_DOWN_IDLE 0
#define PLAYER_DOWN_MOVE_0 1
#define PLAYER_DOWN_MOVE_1 2
#define PLAYER_DOWN_MOVE_2 3
#define PLAYER_DOWN_MOVE_3 4
#define PLAYER_UP_IDLE 5
#define PLAYER_UP_MOVE_0 6
#define PLAYER_UP_MOVE_1 7
#define PLAYER_UP_MOVE_2 8
#define PLAYER_UP_MOVE_3 9

#define PLAYER_IS_UP 0
#define PLAYER_IS_RIGHT 1
#define PLAYER_IS_DOWN 2
#define PLAYER_IS_LEFT 3

#define MAX_TONGUE_FRAMES 20

#define SCREW_SPRITE_OFFSET 11

#define SCREW_IDLE_SPRITE_0 SCREW_SPRITE_OFFSET
#define SCREW_IDLE_SPRITE_1 SCREW_SPRITE_OFFSET + 1

#define SCREW_LAUNCH_SPRITE_0 SCREW_SPRITE_OFFSET + 2
#define SCREW_LAUNCH_SPRITE_1 SCREW_SPRITE_OFFSET + 3
#define SCREW_LAUNCH_SPRITE_2 SCREW_SPRITE_OFFSET + 4

#define SCREW_SIZE 32

#define SCREW_COUNT 50

#define GROUND_SPRITE_OFFSET 15

#define BOSS_SPRITE_0 30
#define BOSS_SPRITE_1 31
#define BOSS_SPRITE_2 32

#define BOMB_SPRITE BOSS_SPRITE_0 + 3
#define CAVE_SPRITE BOMB_SPRITE + 1

// player height: 48px
typedef struct
{
	C2D_Sprite spr;
	float x, y;		  // position
	int spriteNumber; // Number of the sprite in sprite sheet
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
	bool inBossFight;
	int bossFightStartTimer;
	int health;
	int iFrames;
} Player;

typedef struct
{
	C2D_Sprite spr;
	float x, y;		  // position
	int spriteNumber; // Number of the sprite in sprite sheet
	float dx, dy;
	float moveAngle;
	bool alive;
} Bomb;

typedef struct
{
	C2D_Sprite sprite;	// Sprite
	float dx, dy;		// velocity
	float x, y;			// position
	int w, h;			// width, height
	int animationFrame; // Animation frame to use
	int frameTime;		// Frames until next animation
	int iFrames;
	int health;
	int attackFrames;
} Boss;

typedef struct
{
	C2D_Sprite sprite; // Sprite
	bool alive;
	int animationFrame; // Animation frame to use
	int frameTime;		// Frames until next animation
	float dx, dy;		// velocity
	float x, y;			// position
	bool isLaunching;
	int lauchFrame;
	bool stuck;
} ScrewEnemy;

static C2D_SpriteSheet spriteSheet;
static ScrewEnemy screws[SCREW_COUNT];
static Sprite groundTiles[16][10];
static Sprite hearts[3];
static Bomb bomb;
static Player player;
static Boss boss;
static C2D_Sprite cave;
static int frame;
static Sprite win;

static float cameraX, cameraY; // x & y of camera's center
// screen 12.5 x 7.5
//+5 SQUARES OF VIEW when stuck on block
// 12, 12, 12, 12, 12, 7, 9, 9, 9, 9, 10,
// 10, 9, 9, 9, 9, 8, 12, 12, 12, 12, 12,
static int mapLayout[75][64] = {
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{9, 9, 9, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 10, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 9, 9, 9, 9, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 10, 9, 9, 10, 9, 9, 9, 9, 9, 9, 9, 10, 9, 9, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 9, 9, 9, 9, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 10, 9, 9, 10, 9, 9, 9, 9, 9, 9, 9, 10, 9, 9, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 9, 9, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 10, 9, 9, 10, 9, 9, 9, 9, 9, 9, 9, 10, 9, 9, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 9, 9, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 10, 10, 10, 10, 9, 9, 9, 9, 9, 9, 9, 10, 9, 9, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 9, 9, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 9, 9, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 9, 11, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 9, 9, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

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

	for (size_t i = 0; i < 3; i++)
	{
		Sprite *heart = &hearts[i];
		C2D_SpriteSetPos(&heart->spr, heart->x, heart->y);
		heart->x = 18 + (28 * i);
		heart->y = 18;

		C2D_SpriteFromSheet(&heart->spr, spriteSheet, GROUND_SPRITE_OFFSET + 14);
		C2D_SpriteSetCenter(&heart->spr, 0.5f, 0.5f);
		C2D_SpriteSetPos(&heart->spr, heart->x, heart->y);
	}

	p->dx = 0.0f;
	p->dy = 0.0f;
	p->x = (32 * 64) / 2;
	p->y = (32 * 75) / 2;
	p->tongueOut = false;
	p->animationFrame = PLAYER_DOWN_IDLE;
	p->frameTime = 60;
	p->health = 3;
	p->iFrames = 250;
}
// i sneezed
// say bless you

static void initBoss()
{

	player.x = SCREEN_WIDTH / 2;
	player.y = SCREEN_HEIGHT / 2;
	player.dx = 0;
	player.dy = 0;

	Boss *b = &boss;

	C2D_SpriteFromSheet(&b->sprite, spriteSheet, BOSS_SPRITE_0);
	C2D_SpriteSetCenter(&b->sprite, 0.5f, 0.5f);
	C2D_SpriteSetPos(&b->sprite, (SCREEN_WIDTH / 2), (SCREEN_HEIGHT / 2) - 40);

	b->dx = 0.0f;
	b->dy = 0.0f;
	b->x = (SCREEN_WIDTH / 2);
	b->y = (SCREEN_HEIGHT / 2) - 40;
	b->animationFrame = BOSS_SPRITE_0;
	b->frameTime = 60;
	b->health = 3;
	b->iFrames = 30;
	b->attackFrames = 300;

	Bomb *bo = &bomb;
	C2D_SpriteFromSheet(&bo->spr, spriteSheet, BOMB_SPRITE);
	C2D_SpriteSetCenter(&bo->spr, 0.5f, 0.5f);
	C2D_SpriteSetPos(&bo->spr, 0, 0);
	bo->dx = 0.0f;
	bo->dy = 0.0f;
	bo->x = 0;
	bo->y = 0;
	bo->moveAngle = 0;
	bo->alive = false;

	C2D_SpriteFromSheet(&cave, spriteSheet, CAVE_SPRITE);
	C2D_SpriteSetCenter(&cave, 0.0f, 0.0f);
	C2D_SpriteSetPos(&cave, 0, 0);
}

static void initScrews()
{
	for (size_t i = 0; i < SCREW_COUNT; i++)
	{
		ScrewEnemy *screw = &screws[i];

		screw->x = (rand() % (32 * 50));
		screw->y = (rand() % (32 * 50));

		C2D_SpriteFromSheet(&screw->sprite, spriteSheet, SCREW_IDLE_SPRITE_0);
		C2D_SpriteSetCenter(&screw->sprite, 0.5f, 0.5f);
		C2D_SpriteSetPos(&screw->sprite, screw->x, screw->y);
		// C2D_SpriteSetRotation(&screw->sprite, C3D_Angle(rand()/(float)RAND_MAX));
		screw->dx = 0.0;
		screw->dy = 0.0;

		screw->lauchFrame = 0;
		screw->isLaunching = false;
		screw->frameTime = 0;
		screw->alive = true;
		screw->stuck = false;
	}
}

static void initGroundTiles()
{
	for (size_t i = 0; i < 16; i++)
	{
		for (size_t j = 0; j < 10; j++)
		{
			Sprite *tile = &groundTiles[i][j];

			tile->x = player.x - (8 * 32) + (32 * i);
			tile->y = player.y - (5 * 32) + (32 * j);
			// GROUND_SPRITE_OFFSET + 9
			// GROUND_SPRITE_OFFSET + mapLayout[(int)(tile->y / 32)][(int)(tile->x / 32)]

			C2D_SpriteFromSheet(&tile->spr, spriteSheet, 20);
			C2D_SpriteSetCenter(&tile->spr, 0.5f, 0.5f);

			C2D_SpriteSetPos(&tile->spr, tile->x, tile->y);
		}
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

static void checkPlayerCollisions()
{
	float x = player.x + player.dx;
	float y = player.y + player.dy;

	for (size_t i = 0; i < 16; i++)
	{
		for (size_t j = 0; j < 10; j++)
		{
			if (groundTiles[i][j].spriteNumber == GROUND_SPRITE_OFFSET + 10)
			{
				if (x < groundTiles[i][j].x + 32 && x + 48 > groundTiles[i][j].x && player.y < groundTiles[i][j].y + 32 && player.y + 48 > groundTiles[i][j].y)
				{
					player.dx = 0.0;
				}
				if (y < groundTiles[i][j].y + 32 && y + 48 > groundTiles[i][j].y && player.x < groundTiles[i][j].x + 32 && player.x + 48 > groundTiles[i][j].x)
				{
					player.dy = 0.0;
				}
				if (player.dx == 0 && player.dy == 0)
				{
					i = 16;
					j = 10;
					break;
				}
			}
			else if (groundTiles[i][j].spriteNumber == GROUND_SPRITE_OFFSET + 11)
			{
				if (x < groundTiles[i][j].x + 32 && x + 48 > groundTiles[i][j].x && y < groundTiles[i][j].y + 32 && y + 48 > groundTiles[i][j].y)
				{
					player.inBossFight = true;
					initBoss();
					i = 16;
					j = 10;
					break;
				}
			}
		}
	}
}

static void playerFrame()
{
	Player *p = &player;

	if (player.iFrames > 0)
	{
		player.iFrames--;
	}

	p->frameTime--;
	if (p->frameTime == 0)
	{
		if (p->facing == PLAYER_IS_DOWN)
		{
			if (p->dy != 0 || p->dx != 0)
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
		else
		{
			if (p->dy != 0)
			{

				if ((p->animationFrame >= PLAYER_UP_MOVE_3) || (p->animationFrame < PLAYER_UP_MOVE_0))
				{
					C2D_SpriteFromSheet(&p->sprite, spriteSheet, PLAYER_UP_MOVE_0);
					p->animationFrame = PLAYER_UP_MOVE_0;
				}
				else
				{
					p->animationFrame++;
					C2D_SpriteFromSheet(&p->sprite, spriteSheet, p->animationFrame);
				}
			}
			else
			{
				C2D_SpriteFromSheet(&p->sprite, spriteSheet, PLAYER_UP_IDLE);
				p->animationFrame = PLAYER_UP_IDLE;
			}
		}
		p->frameTime = 12;
		C2D_SpriteSetCenter(&p->sprite, 0.5f, 0.5f);
	}

	p->y = p->y + p->dy;
	p->x = p->x + p->dx;
	cameraX = clamp(p->x, SCREEN_WIDTH - 128, 32 * 60);
	cameraY = clamp(p->y, SCREEN_HEIGHT, 32 * 70);

	if (!player.inBossFight)
	{
		C2D_SpriteSetPos(&p->sprite, p->x + getCameraXOffset(), p->y + getCameraYOffset());
	}
	else
	{
		p->y = clamp(player.y, 26, 215);
		p->x = clamp(player.x, 11 + 24, 380);
		C2D_SpriteSetPos(&p->sprite, p->x, p->y);
	}
}

static void screwFrame()
{
	for (size_t i = 0; i < SCREW_COUNT; i++)
	{
		ScrewEnemy *screw = &screws[i];
		if (screw->alive && abs(screw->x - player.x) < 400 && abs(screw->y - player.y) < 400)
		{
			if (!(screw->isLaunching) && (rand() % 600) == 4)
			{
				screw->isLaunching = true;
				screw->lauchFrame = 60;
				C2D_SpriteFromSheet(&screw->sprite, spriteSheet, SCREW_LAUNCH_SPRITE_0);
				screw->animationFrame = SCREW_LAUNCH_SPRITE_0;
				if (abs(screw->x - player.x) < 20)
				{
					screw->dx = 0.0;
				}
				else if ((screw->x - player.x) < 0)
				{
					screw->dx = 4.0;
				}
				else
				{
					screw->dx = -4.0;
				}

				if (abs(screw->y - player.y) < 20)
				{
					screw->dy = 0.0;
				}
				else if ((screw->y - player.y) < 0)
				{
					screw->dy = 4.0;
				}
				else
				{
					screw->dy = -4.0;
				}
			}

			if (screw->isLaunching)
			{
				screw->lauchFrame--;
				if (screw->lauchFrame == 0)
				{
					screw->isLaunching = false;
					screw->dx = 0.0;
					screw->dy = 0.0;
					screw->frameTime = 0;
					screw->animationFrame = SCREW_IDLE_SPRITE_0;
					C2D_SpriteFromSheet(&screw->sprite, spriteSheet, screw->animationFrame);
					C2D_SpriteSetCenter(&screw->sprite, 0.5f, 0.5f);
				}
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
			if (screw->alive && !screw->stuck && player.iFrames == 0 && player.x < screw->x + 32 && player.x + 48 > screw->x && player.y < screw->y + 32 && player.y + 48 > screw->y)
			{
				player.health--;
				player.iFrames = 250;
				screw->alive = false;
			}
			if (player.tongueOut && !screw->stuck && player.x + player.tongueX >= screw->x && player.x + player.tongueX <= screw->x + 32 && player.y + 5 + player.tongueY >= screw->y && player.y + 5 + player.tongueY <= screw->y + 32)
			{
				screw->stuck = true;
			}
			if (screw->stuck)
			{
				screw->x = player.tongueX + player.x - 16;
				screw->y = player.tongueY + player.y + 5 - 16;
				if (!player.tongueOut)
				{
					screw->alive = false;
					player.health = clamp(player.health + 1, 0, 3);
				}
			}
		}
		C2D_SpriteSetPos(&screw->sprite, screw->x + getCameraXOffset(), screw->y + getCameraYOffset());
	}
}

static void bossFrame()
{

	Boss *b = &boss;

	if (!(bomb.alive) && (rand() % 60) == 4)
	{
		bomb.alive = true;
		bomb.moveAngle = atan2(player.y - boss.y, player.x - boss.x);

		bomb.x = boss.x;
		bomb.y = boss.y;
	}

	if (bomb.alive)
	{
		bomb.x += 2 * cos(bomb.moveAngle);
		bomb.y += 2 * sin(bomb.moveAngle);

		C2D_SpriteSetPos(&bomb.spr, bomb.x, bomb.y);

		if (player.iFrames == 0 && player.x < bomb.x + 24 && player.x + 48 > bomb.x && player.y < bomb.y + 24 && player.y + 48 > bomb.y)
		{
			player.health--;
			player.iFrames = 250;
			bomb.alive = false;
		}

		if (bomb.x < -24 || bomb.x > 424 || bomb.y < -24 || bomb.y > 264)
		{
			bomb.alive = false;
		}
	}

	boss.x = (SCREEN_WIDTH / 2) + 40 * sin(frame);
	boss.y = (SCREEN_HEIGHT / 2) - 40;

	if (player.tongueOut && boss.iFrames == 0 && player.x + player.tongueX >= boss.x && player.x + player.tongueX <= boss.x + 89 && player.y + 5 + player.tongueY >= boss.y && player.y + 5 + player.tongueY <= boss.y + 73)
	{
		boss.iFrames = 250;
		boss.health--;
		boss.animationFrame = BOSS_SPRITE_2;
		boss.frameTime = 250;
		C2D_SpriteFromSheet(&b->sprite, spriteSheet, b->animationFrame);
		C2D_SpriteSetCenter(&b->sprite, 0.5f, 0.5f);
		C2D_SpriteSetPos(&b->sprite, boss.x, boss.y);
	}

	if (boss.iFrames > 0)
	{
		boss.iFrames--;
	}

	boss.frameTime--;
	if (boss.frameTime == 0)
	{
		if (boss.animationFrame == BOSS_SPRITE_0)
		{
			boss.animationFrame = BOSS_SPRITE_1;
		}
		else
		{
			boss.animationFrame = BOSS_SPRITE_0;
		}
		C2D_SpriteFromSheet(&b->sprite, spriteSheet, b->animationFrame);
		b->frameTime = 12;
		C2D_SpriteSetCenter(&b->sprite, 0.5f, 0.5f);

		b->y = b->y + b->dy;
		b->x = b->x + b->dx;

		C2D_SpriteSetPos(&b->sprite, b->x, b->y);
	}
}

static void drawGroundTiles()
{
	for (size_t i = 0; i < 16; i++)
	{
		for (size_t j = 0; j < 10; j++)
		{

			if ((groundTiles[i][j].x - cameraX) > (8 * 32))
			{
				groundTiles[i][j].x -= 16 * 32;
				groundTiles[i][j].spriteNumber = GROUND_SPRITE_OFFSET + mapLayout[(int)(groundTiles[i][j].y / 32)][(int)(groundTiles[i][j].x / 32)];
			}
			else if ((cameraX - groundTiles[i][j].x) > (8 * 32))
			{
				groundTiles[i][j].x += 16 * 32;
				groundTiles[i][j].spriteNumber = GROUND_SPRITE_OFFSET + mapLayout[(int)(groundTiles[i][j].y / 32)][(int)(groundTiles[i][j].x / 32)];
			}

			if ((groundTiles[i][j].y - cameraY) > (5 * 32))
			{
				groundTiles[i][j].y -= 10 * 32;
				groundTiles[i][j].spriteNumber = GROUND_SPRITE_OFFSET + mapLayout[(int)(groundTiles[i][j].y / 32)][(int)(groundTiles[i][j].x / 32)];
			}
			else if ((cameraY - groundTiles[i][j].y) > (5 * 32))
			{
				groundTiles[i][j].y += 10 * 32;
				groundTiles[i][j].spriteNumber = GROUND_SPRITE_OFFSET + mapLayout[(int)(groundTiles[i][j].y / 32)][(int)(groundTiles[i][j].x / 32)];
			}

			C2D_SpriteFromSheet(&groundTiles[i][j].spr, spriteSheet, groundTiles[i][j].spriteNumber);
			C2D_SpriteSetCenter(&groundTiles[i][j].spr, 0.5f, 0.5f);
			C2D_SpriteSetPos(&groundTiles[i][j].spr, groundTiles[i][j].x + getCameraXOffset(), groundTiles[i][j].y + getCameraYOffset());
			C2D_DrawSprite(&groundTiles[i][j].spr);
		}
	}
}

static void update()
{
	if (!player.inBossFight)
	{
		checkPlayerCollisions();
		screwFrame();
	}
	else
	{
		bossFrame();
	}
	playerFrame();
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

	// printf("Hello, world!\n");
	// printf("I am sandwhich\n");

	// Main loop
	while (aptMainLoop())
	{

		frame++;
		hidScanInput();

		// User input
		u32 kDown = hidKeysDown();

		if (kDown & KEY_START) {
			if (player.health == 0) {
				init();
			} else {
				break; // break in order to return to hbmenu
			}
		}
			

		u32 kHeld = hidKeysHeld();
		if ((kHeld & KEY_UP))
		{
			(&player)->dy = clamp(player.dy - 0.5, -3.0f, 3.0f);
			if (!player.tongueOut)
			{
				player.facing = PLAYER_IS_UP;
			}
		}
		else if ((kHeld & KEY_DOWN))
		{
			(&player)->dy = clamp(player.dy + 0.5, -3.0f, 3.0f);
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
			(&player)->dx = clamp(player.dx + 0.5, -3.0f, 3.0f);
			if (!player.tongueOut)
			{
				player.facing = PLAYER_IS_DOWN;
			}
		}
		else if ((kHeld & KEY_LEFT))
		{
			(&player)->dx = clamp(player.dx - 0.5, -3.0f, 3.0f);
			if (!player.tongueOut)
			{
				player.facing = PLAYER_IS_DOWN;
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
				player.facing = PLAYER_IS_DOWN;
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
				player.facing = PLAYER_IS_DOWN;
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

		if (!(player.inBossFight && boss.health == 0) && player.health != 0)
			update();

		// Render
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
		C2D_SceneBegin(top);

		// Draw sprites

		if (!player.inBossFight)
			drawGroundTiles();
		else
			C2D_DrawSprite(&cave);

		// Draw Screws
		if (!player.inBossFight)
			for (size_t i = 0; i < SCREW_COUNT; i++)
			{
				if (screws[i].alive && abs(screws[i].x - player.x) < 400 && abs(screws[i].y - player.y) < 400)
				{
					C2D_DrawSprite(&screws[i].sprite);
				}
			}
		else
		{
			C2D_DrawSprite(&boss.sprite);
			if (bomb.alive)
			{
				C2D_DrawSprite(&bomb.spr);
			}
		}

		// Draw player & tongue
		if (player.tongueOut)
		{
			if (!player.inBossFight)
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
				if (player.facing == PLAYER_IS_UP)
				{
					C2D_DrawLine(player.x, player.y - 3, C2D_Color32(255, 80, 80, 200), player.tongueX + player.x, player.tongueY - 3 + player.y, C2D_Color32(255, 80, 80, 255), 3, 0);
					C2D_DrawSprite(&player.sprite);
				}
				else
				{
					C2D_DrawSprite(&player.sprite);
					C2D_DrawLine(player.x, player.y - 3, C2D_Color32(255, 80, 80, 200), player.tongueX + player.x, player.tongueY - 3 + player.y, C2D_Color32(255, 80, 80, 255), 3, 0);
				}
			}
		}
		else
		{
			C2D_DrawSprite(&player.sprite);
		}

		for (size_t i = 0; i < 3; i++)
		{
			if (player.health - 1 < i)
			{
				C2D_SpriteFromSheet(&hearts[i].spr, spriteSheet, GROUND_SPRITE_OFFSET + 13);
			}
			else
			{
				C2D_SpriteFromSheet(&hearts[i].spr, spriteSheet, GROUND_SPRITE_OFFSET + 14);
			}
			C2D_SpriteSetPos(&hearts[i].spr, 18 + (28 * i), 18);
			C2D_SpriteSetCenter(&hearts[i].spr, 0.5f, 0.5f);
			C2D_DrawSprite(&hearts[i].spr);
		}

		//  for (size_t i = 0; i < 3; i++)
		//  {
		//  	C2D_DrawSprite(&hearts[i].spr);
		//  }

		if (player.inBossFight && boss.health == 0)
		{
			C2D_SpriteFromSheet(&win.spr, spriteSheet, CAVE_SPRITE + 1);
			C2D_SpriteSetCenter(&win.spr, 0.0f, 0.0f);
			C2D_SpriteSetPos(&win.spr, 0, 0);
			C2D_DrawSprite(&win.spr);
			// while (difftime(theTime, time(NULL)) < 5)
			// {
			// }

			// break;
		}
		if (player.health == 0)
		{
			C2D_SpriteFromSheet(&win.spr, spriteSheet, CAVE_SPRITE + 2);
			C2D_SpriteSetCenter(&win.spr, 0.0f, 0.0f);
			C2D_SpriteSetPos(&win.spr, 0, 0);
			C2D_DrawSprite(&win.spr);
			// while (difftime(theTime, time(NULL)) < 5)
			// {
			// }

			// break;
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
