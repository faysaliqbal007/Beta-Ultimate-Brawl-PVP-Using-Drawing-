
#define _CRT_SECURE_NO_WARNINGS
#include "iGraphics.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Screen dimensions (full screen)
#define SCREEN_WIDTH  1366 // Adjust to your monitor resolution
#define SCREEN_HEIGHT 768  // Adjust to your monitor resolution
#define MAX_FIREBALLS 10
#define MAX_ITEMS 5

// Game states
enum GameState { HOME, MAIN_MENU, PLAY, TRAINING, ABOUT_US, FIGHT, GAME_OVER, NAME_ENTRY, CHAR_SELECT, MAP_SELECT, TIME_SELECT };
GameState game_state = HOME;

// Character types
enum CharacterType { WARRIOR, MAGE, ARCHER, ASSASSIN, TANK, KNIGHT };
const char* char_names[] = { "Warrior", "Mage", "Archer", "Assassin", "Tank", "Knight" };
const int TOTAL_CHARS = 6;

// Fireball structure
typedef struct {
	int x, y;
	bool active;
	bool right;
	int owner;
} Fireball;

Fireball fireballs[MAX_FIREBALLS];

// Item structure
typedef struct {
	int x, y;
	bool active;
	int type; // 0: Health (+20), 1: Speed (+2), 2: Damage (+5)
} Item;

Item items[MAX_ITEMS];

// Player properties
typedef struct {
	int x, y;
	int health;
	int max_health;
	int attack_dmg;
	int special_dmg;
	int fireball_dmg;
	int last_attack;
	int last_special;
	int last_fireball;
	int last_ability;
	int last_hit_time;
	int combo_count;
	bool attacking;
	bool special;
	bool ability_active;
	bool facing_right;
	int score;
	CharacterType type;
	char name[20];
	int damage_dealt;
	int fireballs_fired;
	float speed_multiplier;
	float damage_multiplier;
	int shield;
	int ability_cooldown;
	int ability_duration;
	int shake_timer; // Individual shake timer for each player
} Player;

Player p1 = { 200, SCREEN_HEIGHT / 2 - 50, 100, 100, 10, 20, 15, 0, 0, 0, 0, 0, 0, false, false, false, true, 0, WARRIOR, "Player 1", 0, 0, 1.0f, 1.0f, 0, 0, 0, 0 };
Player p2 = { 600, SCREEN_HEIGHT / 2 - 50, 100, 100, 10, 20, 15, 0, 0, 0, 0, 0, 0, false, false, false, false, 0, WARRIOR, "Player 2", 0, 0, 1.0f, 1.0f, 0, 0, 0, 0 };

// Game properties
int player_speed = 5;
int attack_range = 60;
int special_range = 80;
int fireball_range = 100;
int fireball_speed = 8;
int attack_cooldown = 800;
int special_cooldown = 1500;
int fireball_cooldown = 1000;
int ability_cooldown = 20000;
int fight_timer = 60;
bool game_over = false;
int winner = 0;

// Event system
bool event_active = false;
int event_type = 0;
int event_timer = 0;
int event_duration = 3;
int event_cooldown = 0;

// Time selection
int selected_time = 0;
const int time_options[] = { 60, 120, 300, 600 };
const char* time_labels[] = { "1 Min", "2 Min", "5 Min", "10 Min" };
int time_btn_w = SCREEN_WIDTH / 6, time_btn_h = SCREEN_HEIGHT / 12;

// Character selection
int selected_char = -1;
bool p1_selected = false, p2_selected = false;

// Button coordinates (scaled for full screen)
int btn_w = SCREEN_WIDTH / 4, btn_h = SCREEN_HEIGHT / 10;
int home_btn_x = SCREEN_WIDTH / 2 - btn_w / 2, home_btn_y = SCREEN_HEIGHT / 2 - btn_h / 2;
int menu_btn_x = SCREEN_WIDTH / 2 - btn_w / 2, menu_btn_y_base = SCREEN_HEIGHT / 2 + btn_h * 1.5;
int char_btn_w = SCREEN_WIDTH / 10, char_btn_h = SCREEN_HEIGHT / 5;
int map_btn_w = SCREEN_WIDTH / 20, map_btn_h = SCREEN_HEIGHT / 10;
int back_btn_x = 50, back_btn_y = 50, resume_btn_x = SCREEN_WIDTH - 250, resume_btn_y = 50;

// Name entry
char p1_name_input[20] = "";
char p2_name_input[20] = "";
bool entering_p1_name = true;

// Map selection
int selected_map = 0;
int map_colors[10][3] = {
	{ 50, 50, 50 },    // Gray
	{ 0, 128, 0 },     // Forest Green
	{ 0, 0, 139 },     // Dark Blue
	{ 139, 69, 19 },   // Brown
	{ 255, 215, 0 },   // Gold
	{ 75, 0, 130 },    // Indigo
	{ 255, 69, 0 },    // Orange Red
	{ 128, 0, 128 },   // Purple
	{ 255, 20, 147 },  // Deep Pink
	{ 0, 191, 255 }    // Sky Blue
};

// Unlockables
int total_score = 0;
int unlocked_chars = 5;
int unlocked_maps = 10;

// Visual effects
int combo_timer = 0;

// Play mode animation
int glow_timer = 0;

void initializeFireballs() {
	for (int i = 0; i < MAX_FIREBALLS; i++) {
		fireballs[i].active = false;
	}
}

void initializeItems() {
	for (int i = 0; i < MAX_ITEMS; i++) {
		items[i].active = false;
	}
}

void spawnItem() {
	int active_items = 0;
	for (int i = 0; i < MAX_ITEMS; i++) {
		if (items[i].active) active_items++;
	}
	if (active_items >= 2) return;

	for (int i = 0; i < MAX_ITEMS; i++) {
		if (!items[i].active) {
			items[i].x = rand() % (SCREEN_WIDTH - 40) + 20;
			items[i].y = rand() % (SCREEN_HEIGHT - 100) + 50;
			items[i].type = rand() % 3;
			items[i].active = true;
			break;
		}
	}
}

void spawnFireball(Player* p) {
	bool spawned = false;
	for (int i = 0; i < MAX_FIREBALLS; i++) {
		if (!fireballs[i].active) {
			int spawn_x = p->x + (p->facing_right ? 20 : -20);
			spawn_x = max(5, min(SCREEN_WIDTH - 5, spawn_x)); // Clamp between 5 and SCREEN_WIDTH - 5
			fireballs[i].x = spawn_x;
			fireballs[i].y = p->y;
			fireballs[i].active = true;
			fireballs[i].right = p->facing_right;
			fireballs[i].owner = (p == &p1) ? 1 : 2;
			p->fireballs_fired++;
			p->last_fireball = glutGet(GLUT_ELAPSED_TIME);
			spawned = true;
			break;
		}
	}
	if (!spawned) {
		int oldest = 0;
		for (int i = 1; i < MAX_FIREBALLS; i++) {
			if (fireballs[i].x < fireballs[oldest].x) oldest = i;
		}
		fireballs[oldest].active = false;
		spawnFireball(p);
	}
}

void drawCharacter(Player* p) {
	int x = p->x, y = p->y;
	int arm_offset = p->facing_right ? 20 : -20;
	int leg_offset = 10;

	// Apply shake effect only if this player's shake_timer is active
	x += (p->shake_timer > 0) ? (rand() % 10 - 5) : 0;
	y += (p->shake_timer > 0) ? (rand() % 10 - 5) : 0;

	if (p->ability_active && p->type == ASSASSIN) return;

	switch (p->type) {
	case WARRIOR:
		iSetColor(150, 75, 0);  iFilledRectangle(x - 15, y - 20, 30, 40);
		iSetColor(255, 0, 0);   iFilledCircle(x, y + 30, 10);
		iSetColor(100, 50, 0);  iFilledRectangle(x + arm_offset, y + 10, 10, 20);
		iFilledRectangle(x - arm_offset, y + 10, 10, 20);
		iFilledRectangle(x - leg_offset, y - 20, 10, 30);
		iFilledRectangle(x + leg_offset, y - 20, 10, 30);
		if (p->attacking) {
			iSetColor(255, 0, 0);
			iLine(x + arm_offset, y + 20, x + (p->facing_right ? 40 : -40), y + 20);
		}
		if (p->special) {
			iSetColor(255, 215, 0);
			iFilledCircle(x + (p->facing_right ? 50 : -50), y + 20, 10);
		}
		if (p->ability_active) {
			iSetColor(255, 0, 0);
			iCircle(x, y + 10, 40);
		}
		break;
	case MAGE:
		iSetColor(0, 0, 255);   iFilledRectangle(x - 15, y - 20, 30, 40);
		iSetColor(200, 200, 255); iFilledCircle(x, y + 30, 10);
		iSetColor(0, 0, 200);   iFilledRectangle(x + arm_offset, y + 10, 10, 20);
		iFilledRectangle(x - arm_offset, y + 10, 10, 20);
		iFilledRectangle(x - leg_offset, y - 20, 10, 30);
		iFilledRectangle(x + leg_offset, y - 20, 10, 30);
		if (p->special) {
			iSetColor(0, 255, 255);
			iFilledCircle(x + (p->facing_right ? 60 : -60), y + 20, 15);
		}
		if (p->ability_active) {
			iSetColor(0, 255, 255);
			iCircle(x, y + 10, 35);
		}
		break;
	case ARCHER:
		iSetColor(0, 128, 0);   iFilledRectangle(x - 15, y - 20, 30, 40);
		iSetColor(0, 255, 0);   iFilledCircle(x, y + 30, 10);
		iSetColor(0, 100, 0);   iFilledRectangle(x + arm_offset, y + 10, 10, 20);
		iFilledRectangle(x - arm_offset, y + 10, 10, 20);
		iFilledRectangle(x - leg_offset, y - 20, 10, 30);
		iFilledRectangle(x + leg_offset, y - 20, 10, 30);
		if (p->special) {
			iSetColor(255, 255, 0);
			iLine(x, y + 20, x + (p->facing_right ? 80 : -80), y + 20);
		}
		break;
	case ASSASSIN:
		iSetColor(50, 50, 50);  iFilledRectangle(x - 15, y - 20, 30, 40);
		iSetColor(100, 100, 100); iFilledCircle(x, y + 30, 10);
		iSetColor(30, 30, 30);  iFilledRectangle(x + arm_offset, y + 10, 10, 20);
		iFilledRectangle(x - arm_offset, y + 10, 10, 20);
		iFilledRectangle(x - leg_offset, y - 20, 10, 30);
		iFilledRectangle(x + leg_offset, y - 20, 10, 30);
		if (p->special) {
			iSetColor(200, 200, 200);
			iLine(x, y + 20, x + (p->facing_right ? 70 : -70), y + 20);
		}
		break;
	case TANK:
		iSetColor(150, 150, 150); iFilledRectangle(x - 20, y - 25, 40, 50);
		iSetColor(200, 200, 200); iFilledCircle(x, y + 35, 12);
		iSetColor(100, 100, 100); iFilledRectangle(x + arm_offset, y + 15, 12, 25);
		iFilledRectangle(x - arm_offset, y + 15, 12, 25);
		iFilledRectangle(x - leg_offset, y - 25, 12, 35);
		iFilledRectangle(x + leg_offset, y - 25, 12, 35);
		if (p->special) {
			iSetColor(255, 165, 0);
			iFilledRectangle(x + (p->facing_right ? 40 : -40), y - 25, 20, 50);
		}
		if (p->ability_active) {
			iSetColor(255, 255, 255);
			iCircle(x, y + 10, 45);
		}
		break;
	case KNIGHT:
		iSetColor(100, 100, 100); iFilledRectangle(x - 20, y - 25, 40, 50);
		iSetColor(255, 215, 0);   iFilledCircle(x, y + 35, 12);
		iSetColor(80, 80, 80);    iFilledRectangle(x + arm_offset, y + 15, 12, 25);
		iFilledRectangle(x - arm_offset, y + 15, 12, 25);
		iFilledRectangle(x - leg_offset, y - 25, 12, 35);
		iFilledRectangle(x + leg_offset, y - 25, 12, 35);
		if (p->special) {
			iSetColor(255, 0, 0);
			iFilledCircle(x + (p->facing_right ? 50 : -50), y + 20, 15);
		}
		if (p->ability_active) {
			iSetColor(255, 0, 0);
			iLine(x, y, x + (p->facing_right ? 50 : -50), y);
		}
		break;
	}

	iSetColor(255, 0, 0);
	iFilledRectangle(x - 25, y + 50, 50, 5);
	iSetColor(0, 255, 0);
	iFilledRectangle(x - 25, y + 50, (p->health * 50) / p->max_health, 5);

	if (p->shield > 0) {
		iSetColor(0, 255, 255);
		iFilledRectangle(x - 25, y + 60, (p->shield * 50) / 20, 5);
	}
}

void drawFireballs() {
	for (int i = 0; i < MAX_FIREBALLS; i++) {
		if (fireballs[i].active) {
			iSetColor(255, 69, 0);
			iFilledCircle(fireballs[i].x, fireballs[i].y, 5);
		}
	}
}

void drawItems() {
	for (int i = 0; i < MAX_ITEMS; i++) {
		if (items[i].active) {
			switch (items[i].type) {
			case 0: iSetColor(0, 255, 0); break;
			case 1: iSetColor(0, 0, 255); break;
			case 2: iSetColor(255, 0, 0); break;
			}
			iFilledCircle(items[i].x, items[i].y, 10);
		}
	}
}

void resetGame() {
	p1 = { 200, SCREEN_HEIGHT / 2 - 50, 100, 100, 10, 20, 15, 0, 0, 0, 0, 0, 0, false, false, false, true, 0, WARRIOR, "", 0, 0, 1.0f, 1.0f, 0, 0, 0, 0 };
	p2 = { 600, SCREEN_HEIGHT / 2 - 50, 100, 100, 10, 20, 15, 0, 0, 0, 0, 0, 0, false, false, false, false, 0, WARRIOR, "", 0, 0, 1.0f, 1.0f, 0, 0, 0, 0 };
	fight_timer = 60;
	game_over = false;
	winner = 0;
	game_state = HOME;
	p1_selected = false;
	p2_selected = false;
	strcpy_s(p1_name_input, sizeof(p1_name_input), "");
	strcpy_s(p2_name_input, sizeof(p2_name_input), "");
	entering_p1_name = true;
	initializeFireballs();
	initializeItems();
	event_active = false;
	event_timer = 0;
	event_cooldown = 0;
	combo_timer = 0;
}

void checkWinLose() {
	if (p1.health <= 0 || p2.health <= 0 || fight_timer <= 0) {
		game_over = true;
		game_state = GAME_OVER;
		if (p1.health <= 0) winner = 2;
		else if (p2.health <= 0) winner = 1;
		else if (p1.score > p2.score) winner = 1;
		else if (p2.score > p2.score) winner = 2;
		else winner = 0;

		total_score += p1.score + p2.score;
		if (total_score >= 100 && unlocked_chars < TOTAL_CHARS) {
			unlocked_chars = TOTAL_CHARS;
		}
	}
}

void updateFireballs() {
	for (int i = 0; i < MAX_FIREBALLS; i++) {
		if (fireballs[i].active) {
			fireballs[i].x += fireballs[i].right ? fireball_speed : -fireball_speed;

			// Check boundaries including fireball radius (5 pixels)
			if (fireballs[i].x < -5 || fireballs[i].x > SCREEN_WIDTH + 5 ||
				fireballs[i].y < -5 || fireballs[i].y > SCREEN_HEIGHT + 5) {
				fireballs[i].active = false;
				continue;
			}

			int dx, dy;
			if (fireballs[i].owner == 1) {
				dx = p2.x - fireballs[i].x;
				dy = p2.y - fireballs[i].y;
				if (dx * dx + dy * dy <= fireball_range * fireball_range) {
					int damage = p1.fireball_dmg * p1.damage_multiplier;
					if (p2.shield > 0) {
						p2.shield -= damage;
						if (p2.shield < 0) {
							p2.health += p2.shield;
							p2.shield = 0;
						}
					}
					else {
						p2.health -= damage;
					}
					p1.score += damage;
					p1.damage_dealt += damage;
					fireballs[i].active = false;
					p2.shake_timer = 5; // Only Player 2 shakes
				}
			}
			else if (fireballs[i].owner == 2) {
				dx = p1.x - fireballs[i].x;
				dy = p1.y - fireballs[i].y;
				if (dx * dx + dy * dy <= fireball_range * fireball_range) {
					int damage = p2.fireball_dmg * p2.damage_multiplier;
					if (p1.shield > 0) {
						p1.shield -= damage;
						if (p1.shield < 0) {
							p1.health += p1.shield;
							p1.shield = 0;
						}
					}
					else {
						p1.health -= damage;
					}
					p2.score += damage;
					p2.damage_dealt += damage;
					fireballs[i].active = false;
					p1.shake_timer = 5; // Only Player 1 shakes
				}
			}
		}
	}
}

void updateItems() {
	for (int i = 0; i < MAX_ITEMS; i++) {
		if (items[i].active) {
			int dx = p1.x - items[i].x;
			int dy = p1.y - items[i].y;
			if (dx * dx + dy * dy <= 20 * 20) {
				switch (items[i].type) {
				case 0: p1.health = (p1.health + 20 > p1.max_health) ? p1.max_health : p1.health + 20; break;
				case 1: p1.speed_multiplier = 1.5f; p1.ability_duration = 5 * 10; break;
				case 2: p1.damage_multiplier = 1.5f; p1.ability_duration = 5 * 10; break;
				}
				items[i].active = false;
			}

			dx = p2.x - items[i].x;
			dy = p2.y - items[i].y;
			if (dx * dx + dy * dy <= 20 * 20) {
				switch (items[i].type) {
				case 0: p2.health = (p2.health + 20 > p2.max_health) ? p2.max_health : p2.health + 20; break;
				case 1: p2.speed_multiplier = 1.5f; p2.ability_duration = 5 * 10; break;
				case 2: p2.damage_multiplier = 1.5f; p2.ability_duration = 5 * 10; break;
				}
				items[i].active = false;
			}
		}
	}
}

void activateEvent() {
	if (!event_active && event_cooldown <= 0) {
		event_type = rand() % 2;
		event_active = true;
		event_timer = event_duration * 10;
		event_cooldown = (rand() % 10 + 20) * 10;
		if (event_type == 0) {
			p1.damage_multiplier = 2.0f;
			p2.damage_multiplier = 2.0f;
		}
		else {
			p1.shield = 1000;
			p2.shield = 1000;
		}
	}
}

void drawDecorativeBackground() {
	// Gradient background
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		float t = (float)y / SCREEN_HEIGHT;
		iSetColor(50 + (int)(100 * t), 50 + (int)(100 * t), 150 + (int)(100 * t));
		iLine(0, y, SCREEN_WIDTH, y);
	}

	// Gold border
	iSetColor(255, 215, 0);
	iRectangle(5, 5, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10);
}

void drawDecoratedPlayButton(int x, int y, int width, int height) {
	// Gradient fill (green to light green)
	for (int i = 0; i < height; i++) {
		float t = (float)i / height;
		iSetColor(0, 128 + (int)(64 * t), 0);
		iLine(x, y + i, x + width, y + i);
	}

	// White border
	iSetColor(255, 255, 255);
	iRectangle(x - 2, y - 2, width + 4, height + 4);

	// Glowing effect (light yellow outline)
	iSetColor(255, 255, 128);
	iRectangle(x - 4, y - 4, width + 8, height + 8);

	// Button text
	iSetColor(255, 255, 255);
	iText(x + width / 2 - 20, y + height / 2 - 10, "Play", GLUT_BITMAP_HELVETICA_18);
}

void drawPlayModeScreen() {
	// Gradient background (already provided by drawDecorativeBackground)

	// Add a glowing panel for "Play Mode" text
	int panel_x = SCREEN_WIDTH / 2 - 150;
	int panel_y = SCREEN_HEIGHT / 2 - 50;
	int panel_w = 300;
	int panel_h = 100;

	// Gradient panel fill (blue to light blue)
	for (int i = 0; i < panel_h; i++) {
		float t = (float)i / panel_h;
		iSetColor(0, 100 + (int)(100 * t), 255);
		iLine(panel_x, panel_y + i, panel_x + panel_w, panel_y + i);
	}

	// Glowing border
	iSetColor(255, 255, 128);
	iRectangle(panel_x - 4, panel_y - 4, panel_w + 8, panel_h + 8);
	iSetColor(255, 255, 255);
	iRectangle(panel_x - 2, panel_y - 2, panel_w + 4, panel_h + 4);

	// Animated glow effect (pulsing)
	iSetColor(255, 255, 128 + (int)(127 * sin(glow_timer * 0.1))); // Fixed: Removed alpha channel
	iRectangle(panel_x - 6, panel_y - 6, panel_w + 12, panel_h + 12);

	// "Play Mode" text
	iSetColor(255, 255, 255);
	iText(SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2, "Play Mode", GLUT_BITMAP_TIMES_ROMAN_24);

	// Back button
	iSetColor(255, 0, 0);
	iFilledRectangle(back_btn_x, back_btn_y, btn_w / 2, btn_h);
	iSetColor(255, 255, 255);
	iText(back_btn_x + 10, back_btn_y + btn_h / 2 - 10, "Back", GLUT_BITMAP_HELVETICA_18);
}

void iDraw() {
	iClear();
	drawDecorativeBackground();

	if (game_state == HOME) {
		iSetColor(255, 215, 0); // Gold text for title
		iText(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 100, "Beta Ultimate Brawl", GLUT_BITMAP_TIMES_ROMAN_24);
		drawDecoratedPlayButton(home_btn_x, home_btn_y, btn_w, btn_h);
	}
	else if (game_state == MAIN_MENU) {
		iSetColor(255, 255, 255);
		iText(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 100, "Main Menu", GLUT_BITMAP_TIMES_ROMAN_24);

		// Play button (decorated)
		drawDecoratedPlayButton(menu_btn_x, menu_btn_y_base, btn_w, btn_h);
		iSetColor(255, 255, 255);

		// Training button
		iSetColor(0, 0, 255);
		iFilledRectangle(menu_btn_x, menu_btn_y_base - btn_h - 10, btn_w, btn_h);
		iSetColor(255, 255, 255);
		iText(menu_btn_x + btn_w / 2 - 30, menu_btn_y_base - btn_h - 10 + btn_h / 2 - 10, "Training", GLUT_BITMAP_HELVETICA_18);

		// About Us button
		iSetColor(255, 69, 0);
		iFilledRectangle(menu_btn_x, menu_btn_y_base - (btn_h + 10) * 2, btn_w, btn_h);
		iSetColor(255, 255, 255);
		iText(menu_btn_x + btn_w / 2 - 40, menu_btn_y_base - (btn_h + 10) * 2 + btn_h / 2 - 10, "About Us", GLUT_BITMAP_HELVETICA_18);

		// Back button
		iSetColor(255, 0, 0);
		iFilledRectangle(back_btn_x, back_btn_y, btn_w / 2, btn_h);
		iSetColor(255, 255, 255);
		iText(back_btn_x + 10, back_btn_y + btn_h / 2 - 10, "Back", GLUT_BITMAP_HELVETICA_18);
	}
	else if (game_state == PLAY) {
		drawPlayModeScreen();
	}
	else if (game_state == TRAINING) {
		iSetColor(255, 255, 255);
		iText(SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2, "Training Mode", GLUT_BITMAP_TIMES_ROMAN_24);
		iSetColor(255, 0, 0);
		iFilledRectangle(back_btn_x, back_btn_y, btn_w / 2, btn_h);
		iSetColor(255, 255, 255);
		iText(back_btn_x + 10, back_btn_y + btn_h / 2 - 10, "Back", GLUT_BITMAP_HELVETICA_18);
	}
	else if (game_state == ABOUT_US) {
		iSetColor(255, 255, 255);
		iText(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 50, "About Us", GLUT_BITMAP_TIMES_ROMAN_24);
		iText(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2, "Developed by: Faysal Bhaiya", GLUT_BITMAP_HELVETICA_18);
		iText(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 - 30, "Version: 1.0 - March 2025", GLUT_BITMAP_HELVETICA_18);
		iSetColor(255, 0, 0);
		iFilledRectangle(back_btn_x, back_btn_y, btn_w / 2, btn_h);
		iSetColor(255, 255, 255);
		iText(back_btn_x + 10, back_btn_y + btn_h / 2 - 10, "Back", GLUT_BITMAP_HELVETICA_18);
	}
	else if (game_state == NAME_ENTRY) {
		iSetColor(255, 255, 255);
		iText(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT - 200, "Enter Player Names", GLUT_BITMAP_TIMES_ROMAN_24);

		iText(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2 + 50, "Player 1 Name:", GLUT_BITMAP_HELVETICA_18);
		iSetColor(entering_p1_name ? 255 : 200, entering_p1_name ? 255 : 200, entering_p1_name ? 255 : 200);
		iFilledRectangle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 40, 300, 40);
		iSetColor(0, 0, 0);
		iText(SCREEN_WIDTH / 2 + 5, SCREEN_HEIGHT / 2 + 45, p1_name_input, GLUT_BITMAP_HELVETICA_18);

		iSetColor(255, 255, 255);
		iText(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2 - 50, "Player 2 Name:", GLUT_BITMAP_HELVETICA_18);
		iSetColor(!entering_p1_name ? 255 : 200, !entering_p1_name ? 255 : 200, !entering_p1_name ? 255 : 200);
		iFilledRectangle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 60, 300, 40);
		iSetColor(0, 0, 0);
		iText(SCREEN_WIDTH / 2 + 5, SCREEN_HEIGHT / 2 - 55, p2_name_input, GLUT_BITMAP_HELVETICA_18);

		iSetColor(0, 255, 0);
		iFilledRectangle(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 4, 200, 50);
		iSetColor(0, 0, 0);
		iText(SCREEN_WIDTH / 2 - 40, SCREEN_HEIGHT / 4 + 15, "Continue", GLUT_BITMAP_TIMES_ROMAN_24);
	}
	else if (game_state == CHAR_SELECT) {
		iSetColor(255, 255, 255);
		iText(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT - 200, "Select Your Character", GLUT_BITMAP_TIMES_ROMAN_24);
		iText(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2 + 100, p1.name, GLUT_BITMAP_HELVETICA_18);
		iText(3 * SCREEN_WIDTH / 4 - 50, SCREEN_HEIGHT / 2 + 100, p2.name, GLUT_BITMAP_HELVETICA_18);
		for (int i = 0; i < unlocked_chars; i++) {
			iSetColor(p1_selected && p1.type == i ? 0 : 255, 255, p1_selected && p1.type == i ? 0 : 255);
			iFilledRectangle(SCREEN_WIDTH / 4 + i * (SCREEN_WIDTH / 8), SCREEN_HEIGHT / 2, char_btn_w, char_btn_h);
			iSetColor(p2_selected && p2.type == i ? 0 : 255, 255, p2_selected && p2.type == i ? 0 : 255);
			iFilledRectangle(SCREEN_WIDTH / 4 + i * (SCREEN_WIDTH / 8), SCREEN_HEIGHT / 4, char_btn_w, char_btn_h);
			iSetColor(0, 0, 0);
			iText(SCREEN_WIDTH / 4 + 10 + i * (SCREEN_WIDTH / 8), SCREEN_HEIGHT / 2 + 50, (char*)char_names[i], GLUT_BITMAP_HELVETICA_18);
			iText(SCREEN_WIDTH / 4 + 10 + i * (SCREEN_WIDTH / 8), SCREEN_HEIGHT / 4 + 50, (char*)char_names[i], GLUT_BITMAP_HELVETICA_18);
		}
		if (unlocked_chars < TOTAL_CHARS) {
			iSetColor(100, 100, 100);
			iFilledRectangle(SCREEN_WIDTH / 4 + unlocked_chars * (SCREEN_WIDTH / 8), SCREEN_HEIGHT / 2, char_btn_w, char_btn_h);
			iFilledRectangle(SCREEN_WIDTH / 4 + unlocked_chars * (SCREEN_WIDTH / 8), SCREEN_HEIGHT / 4, char_btn_w, char_btn_h);
			iSetColor(255, 255, 255);
			iText(SCREEN_WIDTH / 4 + 20 + unlocked_chars * (SCREEN_WIDTH / 8), SCREEN_HEIGHT / 2 + 50, "Locked", GLUT_BITMAP_HELVETICA_18);
			iText(SCREEN_WIDTH / 4 + 20 + unlocked_chars * (SCREEN_WIDTH / 8), SCREEN_HEIGHT / 4 + 50, "Locked", GLUT_BITMAP_HELVETICA_18);
		}
	}
	else if (game_state == MAP_SELECT) {
		iSetColor(255, 255, 255);
		iText(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 200, "Select Map", GLUT_BITMAP_TIMES_ROMAN_24);

		for (int i = 0; i < unlocked_maps; i++) { // Fixed: Replaced removed_chars with unlocked_maps
			int x = SCREEN_WIDTH / 6 + (i % 5) * (SCREEN_WIDTH / 6);
			int y = (i < 5) ? SCREEN_HEIGHT / 2 + 50 : SCREEN_HEIGHT / 2 - 100;
			iSetColor(map_colors[i][0], map_colors[i][1], map_colors[i][2]);
			iFilledRectangle(x, y, map_btn_w, map_btn_h);
			if (i == selected_map) {
				iSetColor(255, 255, 255);
				iRectangle(x, y, map_btn_w, map_btn_h);
			}
		}

		iSetColor(255, 0, 0);
		iFilledRectangle(back_btn_x, back_btn_y, btn_w / 2, btn_h);
		iSetColor(0, 255, 0);
		iFilledRectangle(resume_btn_x, resume_btn_y, btn_w / 2, btn_h);
		iSetColor(0, 0, 0);
		iText(back_btn_x + 10, back_btn_y + 15, "Back", GLUT_BITMAP_HELVETICA_18);
		iText(resume_btn_x + 10, resume_btn_y + 15, "Next", GLUT_BITMAP_HELVETICA_18);
	}
	else if (game_state == TIME_SELECT) {
		iSetColor(255, 255, 255);
		iText(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT - 200, "Select Match Duration", GLUT_BITMAP_TIMES_ROMAN_24);

		for (int i = 0; i < 4; i++) {
			int x = SCREEN_WIDTH / 5 + i * (SCREEN_WIDTH / 5);
			int y = SCREEN_HEIGHT / 2;
			iSetColor(selected_time == i ? 0 : 255, 255, selected_time == i ? 0 : 255);
			iFilledRectangle(x, y, time_btn_w, time_btn_h);
			iSetColor(0, 0, 0);
			iText(x + 20, y + 15, (char*)time_labels[i], GLUT_BITMAP_HELVETICA_18);
		}

		iSetColor(255, 0, 0);
		iFilledRectangle(back_btn_x, back_btn_y, btn_w / 2, btn_h);
		iSetColor(0, 255, 0);
		iFilledRectangle(resume_btn_x, resume_btn_y, btn_w / 2, btn_h);
		iSetColor(0, 0, 0);
		iText(back_btn_x + 10, back_btn_y + 15, "Back", GLUT_BITMAP_HELVETICA_18);
		iText(resume_btn_x + 10, resume_btn_y + 15, "Start", GLUT_BITMAP_HELVETICA_18);
	}
	else if (game_state == FIGHT) {
		iSetColor(map_colors[selected_map][0], map_colors[selected_map][1], map_colors[selected_map][2]);
		iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

		drawItems();
		drawCharacter(&p1);
		drawCharacter(&p2);
		drawFireballs();

		iSetColor(255, 255, 255);
		char score_text[50];
		sprintf_s(score_text, sizeof(score_text), "%s: %d", p1.name, p1.score);
		iText(50, SCREEN_HEIGHT - 50, score_text, GLUT_BITMAP_HELVETICA_18);
		sprintf_s(score_text, sizeof(score_text), "%s: %d", p2.name, p2.score);
		iText(SCREEN_WIDTH - 150, SCREEN_HEIGHT - 50, score_text, GLUT_BITMAP_HELVETICA_18);

		iSetColor(255, 255, 255);
		char timer_text[10];
		sprintf_s(timer_text, sizeof(timer_text), "Time: %d", fight_timer);
		iText(SCREEN_WIDTH / 2 - 30, SCREEN_HEIGHT - 50, timer_text, GLUT_BITMAP_TIMES_ROMAN_24);

		if (event_active) {
			iSetColor(255, 255, 0);
			if (event_type == 0) {
				iText(SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT - 80, "Double Damage!", GLUT_BITMAP_HELVETICA_18);
			}
			else {
				iText(SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT - 80, "Invincibility!", GLUT_BITMAP_HELVETICA_18);
			}
		}

		if (combo_timer > 0) {
			iSetColor(255, 215, 0);
			char combo_text[20];
			sprintf_s(combo_text, sizeof(combo_text), "Combo x%d!", p1.combo_count > 0 ? p1.combo_count : p2.combo_count);
			iText(SCREEN_WIDTH / 2 - 30, SCREEN_HEIGHT / 2, combo_text, GLUT_BITMAP_HELVETICA_18);
		}

		iSetColor(255, 0, 0);
		iFilledRectangle(back_btn_x, back_btn_y, btn_w / 2, btn_h);
		iSetColor(0, 0, 0);
		iText(back_btn_x + 10, back_btn_y + 15, "Back", GLUT_BITMAP_HELVETICA_18);
	}
	else if (game_state == GAME_OVER) {
		iSetColor(255, 215, 0);
		char result[50];
		if (winner == 1) sprintf_s(result, sizeof(result), "%s Wins!", p1.name);
		else if (winner == 2) sprintf_s(result, sizeof(result), "%s Wins!", p2.name);
		else sprintf_s(result, sizeof(result), "Draw!");
		iText(SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 + 50, result, GLUT_BITMAP_TIMES_ROMAN_24);

		char stats[100];
		sprintf_s(stats, sizeof(stats), "%s: %d Damage, %d Fireballs", p1.name, p1.damage_dealt, p1.fireballs_fired);
		iText(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2, stats, GLUT_BITMAP_HELVETICA_18);
		sprintf_s(stats, sizeof(stats), "%s: %d Damage, %d Fireballs", p2.name, p2.damage_dealt, p2.fireballs_fired);
		iText(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 30, stats, GLUT_BITMAP_HELVETICA_18);
		iText(SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2 - 80, "Press 'R' to Restart", GLUT_BITMAP_HELVETICA_18);
	}
}

void iMouseMove(int mx, int my) { }

void iMouse(int button, int state, int mx, int my) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (game_state == HOME) {
			if (mx >= home_btn_x && mx <= home_btn_x + btn_w && my >= home_btn_y && my <= home_btn_y + btn_h) {
				game_state = MAIN_MENU;
			}
		}
		else if (game_state == MAIN_MENU) {
			if (mx >= menu_btn_x && mx <= menu_btn_x + btn_w && my >= menu_btn_y_base && my <= menu_btn_y_base + btn_h) {
				game_state = PLAY;
			}
			if (mx >= menu_btn_x && mx <= menu_btn_x + btn_w && my >= menu_btn_y_base - btn_h - 10 && my <= menu_btn_y_base - 10) {
				game_state = TRAINING;
			}
			if (mx >= menu_btn_x && mx <= menu_btn_x + btn_w && my >= menu_btn_y_base - (btn_h + 10) * 2 && my <= menu_btn_y_base - (btn_h + 10)) {
				game_state = ABOUT_US;
			}
			if (mx >= back_btn_x && mx <= back_btn_x + btn_w / 2 && my >= back_btn_y && my <= back_btn_y + btn_h) {
				game_state = HOME;
			}
		}
		else if (game_state == PLAY || game_state == TRAINING || game_state == ABOUT_US) {
			if (mx >= back_btn_x && mx <= back_btn_x + btn_w / 2 && my >= back_btn_y && my <= back_btn_y + btn_h) {
				game_state = MAIN_MENU;
			}
			if (game_state == PLAY) {
				game_state = NAME_ENTRY; // Start play mode sequence
			}
		}
		else if (game_state == NAME_ENTRY) {
			if (mx >= SCREEN_WIDTH / 2 && mx <= SCREEN_WIDTH / 2 + 300) {
				if (my >= SCREEN_HEIGHT / 2 + 40 && my <= SCREEN_HEIGHT / 2 + 80) entering_p1_name = true;
				if (my >= SCREEN_HEIGHT / 2 - 60 && my <= SCREEN_HEIGHT / 2 - 20) entering_p1_name = false;
			}
			if (mx >= SCREEN_WIDTH / 2 - 100 && mx <= SCREEN_WIDTH / 2 + 100 && my >= SCREEN_HEIGHT / 4 && my <= SCREEN_HEIGHT / 4 + 50 &&
				strlen(p1_name_input) > 0 && strlen(p2_name_input) > 0) {
				strncpy(p1.name, p1_name_input, sizeof(p1.name) - 1);
				p1.name[sizeof(p1.name) - 1] = '\0'; // Ensure null termination
				strncpy(p2.name, p2_name_input, sizeof(p2.name) - 1);
				p2.name[sizeof(p2.name) - 1] = '\0'; // Ensure null termination
				game_state = CHAR_SELECT;
			}
		}
		else if (game_state == CHAR_SELECT) {
			for (int i = 0; i < unlocked_chars; i++) {
				if (!p1_selected && mx >= SCREEN_WIDTH / 4 + i * (SCREEN_WIDTH / 8) && mx <= SCREEN_WIDTH / 4 + i * (SCREEN_WIDTH / 8) + char_btn_w &&
					my >= SCREEN_HEIGHT / 2 && my <= SCREEN_HEIGHT / 2 + char_btn_h) {
					p1.type = (CharacterType)i;
					p1_selected = true;
				}
				if (!p2_selected && mx >= SCREEN_WIDTH / 4 + i * (SCREEN_WIDTH / 8) && mx <= SCREEN_WIDTH / 4 + i * (SCREEN_WIDTH / 8) + char_btn_w &&
					my >= SCREEN_HEIGHT / 4 && my <= SCREEN_HEIGHT / 4 + char_btn_h) {
					p2.type = (CharacterType)i;
					p2_selected = true;
				}
			}
			if (p1_selected && p2_selected) {
				game_state = MAP_SELECT;
			}
		}
		else if (game_state == MAP_SELECT) {
			for (int i = 0; i < unlocked_maps; i++) {
				int x = SCREEN_WIDTH / 6 + (i % 5) * (SCREEN_WIDTH / 6);
				int y = (i < 5) ? SCREEN_HEIGHT / 2 + 50 : SCREEN_HEIGHT / 2 - 100;
				if (mx >= x && mx <= x + map_btn_w && my >= y && my <= y + map_btn_h) {
					selected_map = i;
				}
			}
			if (mx >= back_btn_x && mx <= back_btn_x + btn_w / 2 && my >= back_btn_y && my <= back_btn_y + btn_h) {
				game_state = CHAR_SELECT;
				p1_selected = false;
				p2_selected = false;
			}
			if (mx >= resume_btn_x && mx <= resume_btn_x + btn_w / 2 && my >= resume_btn_y && my <= resume_btn_y + btn_h) {
				game_state = TIME_SELECT;
			}
		}
		else if (game_state == TIME_SELECT) {
			for (int i = 0; i < 4; i++) {
				int x = SCREEN_WIDTH / 5 + i * (SCREEN_WIDTH / 5);
				int y = SCREEN_HEIGHT / 2;
				if (mx >= x && mx <= x + time_btn_w && my >= y && my <= y + time_btn_h) {
					selected_time = i;
					fight_timer = time_options[i];
				}
			}
			if (mx >= back_btn_x && mx <= back_btn_x + btn_w / 2 && my >= back_btn_y && my <= back_btn_y + btn_h) {
				game_state = MAP_SELECT;
			}
			if (mx >= resume_btn_x && mx <= resume_btn_x + btn_w / 2 && my >= resume_btn_y && my <= resume_btn_y + btn_h) {
				game_state = FIGHT;
			}
		}
		else if (game_state == FIGHT) {
			if (mx >= back_btn_x && mx <= back_btn_x + btn_w / 2 && my >= back_btn_y && my <= back_btn_y + btn_h) {
				game_state = TIME_SELECT;
			}
		}
	}
}

void iKeyboard(unsigned char key) {
	int current_time = glutGet(GLUT_ELAPSED_TIME);
	if (game_state == NAME_ENTRY) {
		if (entering_p1_name && strlen(p1_name_input) < 19) {
			if (key >= 32 && key <= 126) {
				int len = strlen(p1_name_input);
				p1_name_input[len] = key;
				p1_name_input[len + 1] = '\0';
			}
			if (key == 8 && strlen(p1_name_input) > 0) p1_name_input[strlen(p1_name_input) - 1] = '\0';
			if (key == 13) entering_p1_name = false;
		}
		else if (!entering_p1_name && strlen(p2_name_input) < 19) {
			if (key >= 32 && key <= 126) {
				int len = strlen(p2_name_input);
				p2_name_input[len] = key;
				p2_name_input[len + 1] = '\0';
			}
			if (key == 8 && strlen(p2_name_input) > 0) p2_name_input[strlen(p2_name_input) - 1] = '\0';
			if (key == 13 && strlen(p1_name_input) > 0 && strlen(p2_name_input) > 0) {
				strncpy(p1.name, p1_name_input, sizeof(p1.name) - 1);
				p1.name[sizeof(p1.name) - 1] = '\0'; // Ensure null termination
				strncpy(p2.name, p2_name_input, sizeof(p2.name) - 1);
				p2.name[sizeof(p2.name) - 1] = '\0'; // Ensure null termination
				game_state = CHAR_SELECT;
			}
		}
	}
	else if (game_state == FIGHT && !game_over) {
		if (key == 'a') { p1.x -= (int)(player_speed * p1.speed_multiplier); p1.facing_right = false; }
		if (key == 'd') { p1.x += (int)(player_speed * p1.speed_multiplier); p1.facing_right = true; }
		if (key == 'w' && p1.y <= SCREEN_HEIGHT - 100) p1.y += 10;
		if (key == 's' && current_time - p1.last_attack >= attack_cooldown) {
			int dx = p2.x - p1.x, dy = p2.y - p1.y;
			if (dx * dx + dy * dy <= attack_range * attack_range) {
				int damage = p1.attack_dmg * p1.damage_multiplier;
				if (p2.shield > 0) {
					p2.shield -= damage;
					if (p2.shield < 0) {
						p2.health += p2.shield;
						p2.shield = 0;
					}
				}
				else {
					p2.health -= damage;
				}
				p1.last_attack = current_time;
				p1.attacking = true;
				p1.damage_dealt += damage;

				if (current_time - p1.last_hit_time < 500) {
					p1.combo_count++;
					combo_timer = 20;
				}
				else {
					p1.combo_count = 1;
				}
				p1.last_hit_time = current_time;
				p1.score += damage * p1.combo_count;
				p2.shake_timer = 5; // Player 2 shakes on melee hit
			}
		}
		if (key == 'e' && current_time - p1.last_special >= special_cooldown) {
			int dx = p2.x - p1.x, dy = p2.y - p1.y;
			if (dx * dx + dy * dy <= special_range * special_range) {
				int damage = p1.special_dmg * p1.damage_multiplier;
				if (p2.shield > 0) {
					p2.shield -= damage;
					if (p2.shield < 0) {
						p2.health += p2.shield;
						p2.shield = 0;
					}
				}
				else {
					p2.health -= damage;
				}
				p1.last_special = current_time;
				p1.special = true;
				p1.score += damage * p1.combo_count;
				p1.damage_dealt += damage;
				p2.shake_timer = 5; // Player 2 shakes on special hit
			}
		}
		if (key == 'f' && current_time - p1.last_fireball >= fireball_cooldown) {
			spawnFireball(&p1);
		}
		if (key == 'r' && current_time - p1.last_ability >= ability_cooldown) {
			p1.ability_active = true;
			p1.last_ability = current_time;
			p1.ability_duration = 5 * 10;
			switch (p1.type) {
			case WARRIOR: p1.damage_multiplier = 2.0f; break;
			case MAGE: p1.shield = 20; break;
			case ARCHER: break;
			case ASSASSIN: break;
			case TANK: break;
			case KNIGHT: p1.x += p1.facing_right ? 50 : -50; break;
			}
		}
	}
	if (key == 'r' && game_over) resetGame();
	if (key == 'q') exit(0);
}

void iSpecialKeyboard(unsigned char key) {
	int current_time = glutGet(GLUT_ELAPSED_TIME);
	if (game_state == FIGHT && !game_over) {
		if (key == GLUT_KEY_LEFT) { p2.x -= (int)(player_speed * p2.speed_multiplier); p2.facing_right = false; }
		if (key == GLUT_KEY_RIGHT) { p2.x += (int)(player_speed * p2.speed_multiplier); p2.facing_right = true; }
		if (key == GLUT_KEY_UP && p2.y <= SCREEN_HEIGHT - 100) p2.y += 10;
		if (key == GLUT_KEY_DOWN && current_time - p2.last_attack >= attack_cooldown) {
			int dx = p1.x - p2.x, dy = p1.y - p2.y;
			if (dx * dx + dy * dy <= attack_range * attack_range) {
				int damage = p2.attack_dmg * p2.damage_multiplier;
				if (p1.shield > 0) {
					p1.shield -= damage;
					if (p1.shield < 0) {
						p1.health += p1.shield;
						p1.shield = 0;
					}
				}
				else {
					p1.health -= damage;
				}
				p2.last_attack = current_time;
				p2.attacking = true;
				p2.damage_dealt += damage;

				if (current_time - p2.last_hit_time < 500) {
					p2.combo_count++;
					combo_timer = 20;
				}
				else {
					p2.combo_count = 1;
				}
				p2.last_hit_time = current_time;
				p2.score += damage * p2.combo_count;
				p1.shake_timer = 5; // Player 1 shakes on melee hit
			}
		}
		if (key == GLUT_KEY_END && current_time - p2.last_special >= special_cooldown) {
			int dx = p1.x - p2.x, dy = p1.y - p2.y;
			if (dx * dx + dy * dy <= special_range * special_range) {
				int damage = p2.special_dmg * p2.damage_multiplier;
				if (p1.shield > 0) {
					p1.shield -= damage;
					if (p1.shield < 0) {
						p1.health += p1.shield;
						p1.shield = 0;
					}
				}
				else {
					p1.health -= damage;
				}
				p2.last_special = current_time;
				p2.special = true;
				p2.score += damage * p2.combo_count;
				p2.damage_dealt += damage;
				p1.shake_timer = 5; // Player 1 shakes on special hit
			}
		}
		if (key == GLUT_KEY_HOME && current_time - p2.last_fireball >= fireball_cooldown) {
			spawnFireball(&p2);
		}
		if (key == GLUT_KEY_PAGE_DOWN && current_time - p2.last_ability >= ability_cooldown) {
			p2.ability_active = true;
			p2.last_ability = current_time;
			p2.ability_duration = 5 * 10;
			switch (p2.type) {
			case WARRIOR: p2.damage_multiplier = 2.0f; break;
			case MAGE: p2.shield = 20; break;
			case ARCHER: break;
			case ASSASSIN: break;
			case TANK: break;
			case KNIGHT: p2.x += p2.facing_right ? 50 : -50; break;
			}
		}
	}
}

void updateGame() {
	if (game_state == FIGHT && !game_over) {
		if (p1.y > SCREEN_HEIGHT / 2 - 50) p1.y -= 5;
		if (p2.y > SCREEN_HEIGHT / 2 - 50) p2.y -= 5;

		if (glutGet(GLUT_ELAPSED_TIME) - p1.last_attack > attack_cooldown / 2) p1.attacking = false;
		if (glutGet(GLUT_ELAPSED_TIME) - p2.last_attack > attack_cooldown / 2) p2.attacking = false;
		if (glutGet(GLUT_ELAPSED_TIME) - p1.last_special > special_cooldown / 2) p1.special = false;
		if (glutGet(GLUT_ELAPSED_TIME) - p2.last_special > special_cooldown / 2) p2.special = false;

		static int regen_timer = 0;
		regen_timer++;
		if (regen_timer >= 50) {
			if (p1.health < p1.max_health) p1.health += (p1.type == TANK ? 3 : 1);
			if (p2.health < p2.max_health) p2.health += (p2.type == TANK ? 3 : 1);
			regen_timer = 0;
		}

		if (p1.ability_duration > 0) {
			p1.ability_duration--;
			if (p1.ability_duration <= 0) {
				p1.ability_active = false;
				p1.speed_multiplier = 1.0f;
				p1.damage_multiplier = (event_active && event_type == 0) ? 2.0f : 1.0f;
				p1.shield = (event_active && event_type == 1) ? 1000 : 0;
				if (p1.type == TANK) p1.ability_active = true;
			}
		}
		if (p2.ability_duration > 0) {
			p2.ability_duration--;
			if (p2.ability_duration <= 0) {
				p2.ability_active = false;
				p2.speed_multiplier = 1.0f;
				p2.damage_multiplier = (event_active && event_type == 0) ? 2.0f : 1.0f;
				p2.shield = (event_active && event_type == 1) ? 1000 : 0;
				if (p2.type == TANK) p2.ability_active = true;
			}
		}

		if (event_active) {
			event_timer--;
			if (event_timer <= 0) {
				event_active = false;
				p1.damage_multiplier = 1.0f;
				p2.damage_multiplier = 1.0f;
				p1.shield = 0;
				p2.shield = 0;
			}
		}
		if (event_cooldown > 0) event_cooldown--;

		// Decrease shake timers for both players
		if (p1.shake_timer > 0) p1.shake_timer--;
		if (p2.shake_timer > 0) p2.shake_timer--;
		if (combo_timer > 0) combo_timer--;

		updateFireballs();
		updateItems();
		checkWinLose();

		static int item_spawn_timer = 0;
		item_spawn_timer++;
		if (item_spawn_timer >= 200) {
			spawnItem();
			item_spawn_timer = 0;
		}

		static int event_spawn_timer = 0;
		event_spawn_timer++;
		if (event_spawn_timer >= 200) {
			activateEvent();
			event_spawn_timer = 0;
		}
	}

	// Update glow timer for animations
	glow_timer++;
}

void timerUpdate() {
	if (game_state == FIGHT && !game_over) {
		fight_timer--;
		checkWinLose();
	}
}

int main() {
	srand((unsigned int)time(NULL));
	initializeFireballs();
	initializeItems();

	iSetTimer(1000, timerUpdate);
	iSetTimer(100, updateGame);
	iInitialize(SCREEN_WIDTH, SCREEN_HEIGHT, "Beta Ultimate Brawl");

	// Enable full-screen mode using GLUT
	glutFullScreen();

	return 0;
}