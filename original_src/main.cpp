/* Each level of "HotKeyz" contains two parts: The first part is intended to help users learn
 the keyboard short cuts in ProTools (control T, option-shift D, etc). The second part is
 intended to help users learn the mouse-click modifiers in ProTools (command click, option-shift click,
 etc). There are a total of seven levels.*/

#include "SDL2/SDL.h"
#include "SDL2_image/SDL_image.h"
#include "SDL2/SDL_thread.h"
#include "SDL2_ttf/SDL_ttf.h"
#include "SDL2_mixer/SDL_mixer.h"
#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <sstream>
#include <atomic>

using std::cerr;
using std::atomic_bool;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

atomic_bool shutdownThread(false);

SDL_Surface *background[2][4] =
{
	{NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL}
};
SDL_Surface *ptImg[2][12] =
{
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}
};
SDL_Surface *csFrame = NULL;
SDL_Surface *csPowerup = NULL;
SDL_Surface *screen = NULL;
SDL_Surface *scoreTxt = NULL;
SDL_Surface *livesTxt = NULL;
SDL_Surface *chainTxt = NULL;
SDL_Surface *finalTxt = NULL;
SDL_Surface *chooseTxt = NULL;
SDL_Surface *review[7] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
SDL_Surface *scReview[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
SDL_Surface *timeTxt = NULL;
SDL_Surface *continueTxt = NULL;
SDL_Surface *scIcon[36] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
SDL_Surface *mic[2] = {NULL, NULL};
SDL_Surface *scBackground[3] = {NULL, NULL, NULL};
SDL_Surface *warning = NULL;
SDL_Surface *wrongAns = NULL;
SDL_Surface *rightAns = NULL;
SDL_Surface *scClockStop = NULL;
SDL_Surface *keyboardPrompt = NULL;
SDL_Surface *keyboardTypes[2] = {NULL, NULL};
SDL_Surface *endScreen = NULL;
SDL_Surface *wave[5][4] =
{
	{NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL}
};
SDL_Surface *pauseMenu[2] = {NULL, NULL};

SDL_Event event;

SDL_Thread *thread1 = NULL;

Mix_Chunk *failSnd = NULL;
Mix_Chunk *dieSnd = NULL;
Mix_Chunk *succeedSnd = NULL;
Mix_Chunk *csMusic = NULL;
Mix_Chunk *scStartSound[2] = {NULL, NULL};
Mix_Chunk *scGoodSound = NULL;
Mix_Chunk *scBadSound = NULL;
Mix_Chunk *oneUpSnd2 = NULL;
Mix_Chunk *oneUpSnd1 = NULL;

Mix_Music *BGmusic2 = NULL;
Mix_Music *BGmusic1[3] = {NULL, NULL, NULL};
Mix_Music *finaleMusic1[3] = {NULL, NULL, NULL};
Mix_Music *finaleMusic2 = NULL;
Mix_Music *reviewMusic2 = NULL;
Mix_Music *reviewMusic1 = NULL;
Mix_Music *badFinaleMusic1 = NULL;
Mix_Music *badFinaleMusic2 = NULL;

TTF_Font *smallFont = NULL;
TTF_Font *font = NULL;
TTF_Font *bigFont = NULL;

std::stringstream sScore, sLives, sChain, sRemaining, sLvlEndTxt1, sLvlEndTxt2;

SDL_Color textColor = {255, 255, 255};

// global variables

short int lives = 5, scLog = 1;
long int score = 0;
long int oldScore;
short int gr, levelNum, lvlHalf, roundPart;
unsigned int tmpRemain;
float s;
unsigned short int chain = 0;
bool respawn, csAvail, stopClock, showCheck, csLocked, dieScreen, yayScreen, booScreen;
Uint32 gRegenTime, initialTime, check_time, scClockStop_time, dieTime, yayTime, booTime;
short int check_position[2] = {0, 0};
short int cs_position[2];

// global constants

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int SCREEN_BPP = 32;

// TIME_LEVELS determines how many miliseconds long each round is.

const Uint32 TIME_LEVELS[2][7] =
{
	{100000, 100000, 152300, 100000, 100000, 152300, 235800},
	{100000, 100000, 150000, 100000, 100000, 150000, 250000}
};

// KC_LEVELS and SET_LEVELS determine which icons will appear in each "mouse-click modifier" round.

const short int KC_LEVELS[7][2] =
{
	{6, 0},
	{6, 6},
	{12, 0},
	{6, 0},
	{6, 6},
	{12, 0},
	{12, 0}
};

const short int SET_LEVELS[7][2] =
{
	{1, 0},
	{1, 0},
	{1, 0},
	{1, 1},
	{1, 1},
	{1, 1},
	{2, 0}
};

// these four variables handle the difficulty in all the levels of the "mouse-click modifier" rounds
const short int GR_LEVELS[7] = {65, 70, 70, 75, 80, 80, 80};
const float SPEED_LEVELS[7] = {0.02, 0.03, 0.03, 0.04, 0.05, 0.06, 0.06};
const short int GR_START_LEVELS[7] = {4300, 4000, 3800, 3600, 3500, 3400, 3400};
const short int GR_CAP_LEVELS[7] = {1500, 1250, 1250, 1000, 1000, 900, 900};
const float SPD_START_LEVELS[7] = {1, 1, 1.5, 1.5, 2, 2, 2.5};

// CS_CHANCE determines the likelihood that the clockstopper powerup will appear in the "mouse-click modifier" rounds.
// As the generation rate speeds up, smaller numbers are grabbed from this array
// so there is a greater likelihood of getting a powerup.
const short int CS_CHANCE[6] = {20, 30, 50, 70, 90, 100};

// HALFTIME_LEVELS determines at what point the drop speed will increase in the middle of the "keyboard shortcut" rounds
// (only occurs in levels 3, 6, and 7)
const short int HALFTIME_LEVELS[7] = {17, 17, 17, 17, 17, 17, 33};

// SHORTCUT_CS_CHANCE determines the likelyhood of the clockstopper powerup
// in each of the seven "keyboard shortcut" levels.
const short int SHORTCUT_CS_CHANCE[7] = {50, 50, 40, 50, 50, 40, 35};

/* shortcut_codes contains all the keyboard shortcuts for every "keyboard shortcut" level.
shortcut_codes[0][] determines what combination of shift, control, option, and command should be pressed.
shortcut_codes[1][] determines what other key should be pressed.
it's not a constant because it's changed if the user selects the "laptop keyboard" option.*/
short int shortcut_codes[2][36] =
{
	{0, 0, 0, 0, 1, 1, 0, 3, 1, 1, 1, 1, 3, 3, 1, 7, 15, 7, 7, 7, 7, 7, 8, 8, 7, 11, 1, 2, 1, 1, 0, 3, 3, 3, 1, 7},
	{282, 283, 284, 285, 114, 116, 13, 13, 54, 55, 273, 274, 91, 93, 45, 61, 110, 107, 52, 53, 54, 55, 49, 115, 103, 100, 9, 9, 112, 59, 9, 9, 270, 269, 98, 104}
};

// SC_LEVELS determines which shortcut codes are used in each "keyboard shortcut" level.
const short int SC_LEVELS[7][2] =
{
	{8, 0},
	{8, 8},
	{18, 0},
	{8, 18},
	{8, 26},
	{18, 18},
	{36, 0}
};

// the next two constants determine the starting and maximum difficulty in each "keyboard shortcut" level.
const short int SC_DIFFSTART_LEVELS[7] = {15, 15, 20, 15, 15, 20, 17};
const short int SC_DIFFMAX_LEVELS[7] = {30, 30, 39, 39, 40, 40, 49};

// SC_DROPTIME determines the drop time for the icons in each "keyboard shortcut" level.
// the drop times keep the drops in sync with the music.
const int SC_DROPTIME[7][2] =
{
	{3986, 3986},
	{3986, 3986},
	{3986, 3493},
	{3986, 3986},
	{3986, 3986},
	{3986, 3493},
	{3492, 2983}
};

// SC_FINALE_NUMB determines the number of drops defore each "keyboard shortcut" round ends.
const short int SC_FINALE_NUMB[7] = {26, 26, 42, 26, 26, 42, 74};

// MUSIC_SPEED determines which background music to use in the first and second halves
// of each "keyboard shortcut" level.
const short int MUSIC_SPEED[7][2] =
{
	{0, 0},
	{0, 0},
	{0, 1},
	{0, 0},
	{0, 0},
	{0, 1},
	{1, 2}
};

// these three constants provide the coordinates for the icons in the "keyboard shortcut" rounds.
const short int X_ICON[4] = {69, 367, 664, 960};
const short int Y_ICON[5] = {112, 204, 295, 386, 477};
const short int X_MIC[4] = {97, 395, 693, 991};

// function prototypes for core sdl functionality
SDL_Surface *load_image(std::string);
void apply_surface(int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL);
bool init();
bool load_files();
void clean_up();

// the "Shortcuts" struct creates all the objects for the "keyboard shortcut" rounds.
struct Shortcuts
{
	short int m_Code, m_Mod, m_Col, m_Row;
	Uint16 m_Letter;
	Uint32 m_BanTime;
	bool m_Ct, m_Op, m_Cm, m_Sh, m_ShowBan;

	Shortcuts(short int, short int, Uint16, short int);
	void ShowIcon();
	bool CheckInput();
	bool MoveDown();
};

/*
 m_Ct = control key
 m_Op = option key
 m_Cm = command key
 m_Sh = shift key
 m_Letter = the key code of the key that should be pressed along with the modifier key(s)
*/

Shortcuts::Shortcuts(short int code, short int mod, Uint16 letter, short int col)
{
	m_Code = code;
	m_Mod = mod;
	m_Letter = letter;
	m_Col = col;
	m_Row = 0;
	m_ShowBan = false;

	switch (m_Mod)
	{
		case 0:
			m_Ct = false;
			m_Op = false;
			m_Cm = false;
			m_Sh = false;
			break;
		case 1:
			m_Ct = true;
			m_Op = false;
			m_Cm = false;
			m_Sh = false;
			break;
		case 2:
			m_Ct = true;
			m_Op = true;
			m_Cm = false;
			m_Sh = false;
			break;
		case 3:
			m_Ct = false;
			m_Op = true;
			m_Cm = false;
			m_Sh = false;
			break;
		case 4:
			m_Ct = true;
			m_Op = true;
			m_Cm = true;
			m_Sh = false;
			break;
		case 5:
			m_Ct = true;
			m_Op = false;
			m_Cm = true;
			m_Sh = false;
			break;
		case 6:
			m_Ct = false;
			m_Op = true;
			m_Cm = true;
			m_Sh = false;
			break;
		case 7:
			m_Ct = false;
			m_Op = false;
			m_Cm = true;
			m_Sh = false;
			break;
		case 8:
			m_Ct = false;
			m_Op = false;
			m_Cm = false;
			m_Sh = true;
			break;
		case 9:
			m_Ct = true;
			m_Op = false;
			m_Cm = false;
			m_Sh = true;
			break;
		case 10:
			m_Ct = true;
			m_Op = true;
			m_Cm = false;
			m_Sh = true;
			break;
		case 11:
			m_Ct = false;
			m_Op = true;
			m_Cm = false;
			m_Sh = true;
			break;
		case 12:
			m_Ct = true;
			m_Op = true;
			m_Cm = true;
			m_Sh = true;
			break;
		case 13:
			m_Ct = true;
			m_Op = false;
			m_Cm = true;
			m_Sh = true;
			break;
		case 14:
			m_Ct = false;
			m_Op = true;
			m_Cm = true;
			m_Sh = true;
			break;
		case 15:
			m_Ct = false;
			m_Op = false;
			m_Cm = true;
			m_Sh = true;
			break;
		default:
			break;
	}
}

void Shortcuts::ShowIcon()
{
	if ((m_Row == 4) && (((SDL_GetTicks() / 200) % 2) == 0))
	{
		apply_surface(X_ICON[m_Col], Y_ICON[m_Row], warning, screen);
	}
	apply_surface(X_ICON[m_Col], Y_ICON[m_Row], scIcon[m_Code], screen);
	if (m_ShowBan)
	{
		apply_surface(X_ICON[m_Col], Y_ICON[m_Row], wrongAns, screen);
		if ((SDL_GetTicks() - m_BanTime) > 300)
		{
			m_ShowBan = false;
		}
	}
}

bool Shortcuts::CheckInput()
{
	const Uint8 *keystates = SDL_GetKeyboardState(NULL);

	bool chk1 = (m_Ct == (keystates[SDL_SCANCODE_LCTRL] || keystates[SDL_SCANCODE_RCTRL]));
	bool chk2 = (m_Op == (keystates[SDL_SCANCODE_LALT] || keystates[SDL_SCANCODE_RALT]));
	bool chk3 = (m_Cm == (keystates[SDL_SCANCODE_LGUI] || keystates[SDL_SCANCODE_RGUI]));
	bool chk4 = (m_Sh == (keystates[SDL_SCANCODE_LSHIFT] || keystates[SDL_SCANCODE_RSHIFT]));
	bool keyChk = (event.key.keysym.sym == m_Letter);
	bool oll_korrect = ((chk1) && (chk2) && (chk3) && (chk4) && (keyChk));

	if (!oll_korrect)
	{
		m_ShowBan = true;
		m_BanTime = SDL_GetTicks();
	}

	return oll_korrect;
}

bool Shortcuts::MoveDown()
{
	bool offscr;

	if (m_Row < 4)
	{
		m_Row++;
		offscr = false;
	}
	else
	{
		offscr = true;
	}

	return offscr;
}

// the Hotkeys struct creates all the objects for the "mouse-click modifiers" rounds.
struct Hotkeys
{
	short int m_Set, m_Hor, m_Ver, m_Kc, m_Speed, m_CSchance;
	bool m_Ct, m_Op, m_Cm, m_Sh, m_Match, m_OffScr, m_Powerup;

	Hotkeys(short int, short int, short int, short int);
	void ShowTxt();
	bool CheckKeys(short int, short int);
	bool MoveLeft();
};

Hotkeys::Hotkeys(short int set, short int kc, short int ver, short int spd)
{
	m_Set = set;
	m_Hor = 1280;
	m_Ver = ver;
	m_Kc = kc;
	m_Speed = spd;
	m_CSchance = gr / 1000;
	m_Powerup = ((rand() % CS_CHANCE[m_CSchance]) == 1);

	if (m_Powerup)
	{
		m_Speed = -15;
		m_Hor = -250;
	}

	switch (m_Kc)
	{
		case 0:
			m_Ct = true;
			m_Op = false;
			m_Cm = false;
			m_Sh = false;
			break;
		case 1:
			m_Ct = true;
			m_Op = true;
			m_Cm = false;
			m_Sh = false;
			break;
		case 2:
			m_Ct = false;
			m_Op = true;
			m_Cm = false;
			m_Sh = false;
			break;
		case 3:
			m_Ct = true;
			m_Op = true;
			m_Cm = true;
			m_Sh = false;
			break;
		case 4:
			m_Ct = true;
			m_Op = false;
			m_Cm = true;
			m_Sh = false;
			break;
		case 5:
			m_Ct = false;
			m_Op = true;
			m_Cm = true;
			m_Sh = false;
			break;
		case 6:
			m_Ct = false;
			m_Op = false;
			m_Cm = true;
			m_Sh = false;
			break;
		case 7:
			m_Ct = false;
			m_Op = false;
			m_Cm = false;
			m_Sh = true;
			break;
		case 8:
			m_Ct = true;
			m_Op = false;
			m_Cm = false;
			m_Sh = true;
			break;
		case 9:
			m_Ct = false;
			m_Op = true;
			m_Cm = false;
			m_Sh = true;
			break;
		case 10:
			m_Ct = false;
			m_Op = true;
			m_Cm = true;
			m_Sh = true;
			break;
		case 11:
			m_Ct = false;
			m_Op = false;
			m_Cm = true;
			m_Sh = true;
			break;
		default:
			break;
	}
}

void Hotkeys::ShowTxt()
{
	if (m_Powerup)
	{
		apply_surface((m_Hor - 10), (m_Ver - 10), csFrame, screen);
	}
	apply_surface(m_Hor, m_Ver, ptImg[m_Set][m_Kc], screen );
}

bool Hotkeys::CheckKeys(short int h, short int v)
{
	const Uint8 *keystates = SDL_GetKeyboardState(NULL);

	bool chk1 = (m_Ct == (keystates[SDL_SCANCODE_LCTRL] || keystates[SDL_SCANCODE_RCTRL]));
	bool chk2 = (m_Op == (keystates[SDL_SCANCODE_LALT] || keystates[SDL_SCANCODE_RALT]));
	bool chk3 = (m_Cm == (keystates[SDL_SCANCODE_LGUI] || keystates[SDL_SCANCODE_RGUI]));
	bool chk4 = (m_Sh == (keystates[SDL_SCANCODE_LSHIFT] || keystates[SDL_SCANCODE_RSHIFT]));
	bool mousePos =  ((v > (m_Ver + 10)) && (v < (m_Ver + 155)) && (h > (m_Hor + 5)) && (h < (m_Hor + 205)));
	m_Match = ((chk1) && (chk2) && (chk3) && (chk4) && (mousePos));

	return m_Match;
}

bool Hotkeys::MoveLeft()
{
	if (!m_Powerup)
    {
        if (m_Hor > 1080)
        {
            m_Hor -= 20;
        }
        else
        {
            m_Hor -= m_Speed;
        }

        m_OffScr = (m_Hor < -200);
    }
    else
    {
        m_Hor -= m_Speed;
        m_OffScr = (m_Hor > 1270);
    }

	return m_OffScr;
}

bool getKeyboardType();
void showGameData();
void showBG();
void endGameData(unsigned short int);
void increaseDiff();
short int getLevel();
void addShortcutObject(short int);
void showShortcutIcons();
short int dropShortcutIcons();
short int checkShortcut(short int);
void moveMic(short int& rMicPos);
void scScore();
void showShortcutBG();
void showSCclockstop();
void scReviewScreen();
void reviewScreen();
Uint32 openPauseMenu();
int gen_thread(void *data);

std::vector<Hotkeys> hk;
std::vector<Shortcuts> scColumns[4];

int main(int argc, char** argv)
{
	short int keyCombo, vOffset, set, speed, i, x, y, micPos, lostLives, scStatus, badChain, scDiff, scGenRate, dropNumb;
	unsigned short int hiChain;
	unsigned long int csTime = 0;
	int waitTime, remainingTime;
	bool startFinale;
	Uint32 badTime, pauseOffset;
	const Uint8 *keystates = NULL;
	Uint16 nonModifiers[81] = {8, 9, 13, 32, 39, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 59, 61, 91, 92, 93, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 273, 274, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293};
	std::vector<Uint16> lettersNumbers(nonModifiers, nonModifiers + 81);
	std::vector<Uint16>::iterator letIter;

    if (init() == false)
    {
        return 1;
    }

    if (load_files() == false)
    {
        return 1;
    }

	srand(static_cast<unsigned int>(time(0)));

	short int debugLvl = 0;

	// enable these 3 lines of code to skip levels:

	/*apply_surface(10, 300, chooseTxt, screen);
	SDL_RenderPresent(renderer);
	debugLvl = getLevel();*/

	if (getKeyboardType() == false)
	{
		return 1;
	}
    if (event.type == SDL_QUIT)
    {
        clean_up();
        return 0;
    }

	for (levelNum = debugLvl; levelNum < 7; levelNum++)
	{
		roundPart = 0;
		csLocked = false;
		stopClock = false;
		csAvail = false;
        dieScreen = false;
		micPos = 0;
		chain = 0;
		hiChain = 0;
		badChain = 0;
		lvlHalf = 0;
		scDiff = SC_DIFFSTART_LEVELS[levelNum];
		dropNumb = 1;

		scReviewScreen();

        if (event.type == SDL_QUIT)
        {
            clean_up();
            return 0;
        }

		scGenRate = scDiff / 10;
		addShortcutObject((rand() % scGenRate) + 1);
		initialTime = SDL_GetTicks();

		showShortcutBG();
		SDL_RenderPresent(renderer);
		Mix_PlayChannel(-1, scStartSound[MUSIC_SPEED[levelNum][0]], 0);
		SDL_Delay(550);

		Mix_PlayMusic(BGmusic1[MUSIC_SPEED[levelNum][0]], -1);

		/* This is the "keyboard shortcuts" part of the game. Icons that represent ProTools functions
		 appear at the top of the screen, and then, every 3 to 4 seconds, they drop down by one row.
		 You must destroy them before they make it all the way down to the bottom of the screen.  To do this,
		 use the left or right arrow keys to position the shotgun mic directly below a function icon, and
		 then "shoot" the icon by pressing the appropriate key combination for that icon's function.
		 (For example, use option+shift+D to shoot the "Duplicate track" icon, etc.) */

		while ((lives > 0) && ((SDL_GetTicks() - initialTime) < TIME_LEVELS[0][levelNum]))
		{
			waitTime = SDL_GetTicks();
			remainingTime = (SDL_GetTicks() - waitTime);

			while (remainingTime < SC_DROPTIME[levelNum][lvlHalf])
			{
				/* this while loop contains all the events that occur between icon drops.
				 it lasts between 3 and 4 seconds, depending on the level.
				 also, the drop time will speed up halfway through levels 3, 6, and 7. */

				showShortcutBG();
				showShortcutIcons();
				showSCclockstop();
				showGameData();
				apply_surface(X_MIC[micPos], 574, mic[(SDL_GetTicks() / 500) % 2], screen);
				SDL_RenderPresent(renderer);

				scStatus = 0;
				if (SDL_PollEvent(&event))
				{
                    if (event.type == SDL_QUIT)
                    {
                        clean_up();
                        return 0;
                    }

                    else if (event.type == SDL_KEYDOWN)
                    {

                        keystates = SDL_GetKeyboardState(NULL);

                        if (keystates[SDL_SCANCODE_ESCAPE])
                        {
                            // lets you pause the game
                            pauseOffset = openPauseMenu();
                            if (pauseOffset == 0)
                            {
                                clean_up();
                                return 0;
                            }

                            initialTime += pauseOffset;
                            waitTime += pauseOffset;
                            csTime += pauseOffset;
                        }

                        else if ((keystates[SDL_SCANCODE_LEFT]) || (keystates[SDL_SCANCODE_RIGHT]))
                        {
                            // lets you move the shotgun mic
                            moveMic(micPos);
                        }
                        else if (keystates[SDL_SCANCODE_SPACE])
                        {
                            if (csAvail)
                            {
                                // lets you activate the powerup, if one is available.
                                csTime = SDL_GetTicks();
                                csAvail = false;
                                tmpRemain = ((TIME_LEVELS[0][levelNum] + 999) - (SDL_GetTicks() - initialTime)) / 1000;
                                initialTime += 6100;
                                waitTime += 6100;
                                stopClock = true;
                                Mix_PauseMusic();
                                Mix_PlayChannel(3, csMusic, 0);
                            }
                        }
                        else
                        {
                            for (letIter = lettersNumbers.begin(); letIter < lettersNumbers.end(); letIter++)
                            {
                                if (keystates[*letIter])
                                {
                                    // if you're attempting a keyboard shortcut
                                    oldScore = score;
                                    scStatus = checkShortcut(micPos);
                                    break;
                                }
                            }
                            if (scStatus >= 1)
                            {
                                // if your attempt was successful
                                scScore();

                                // The variable scDiff is for adaptive difficulty.
                                scDiff += (scStatus - 2);
                                if (scDiff > SC_DIFFMAX_LEVELS[levelNum])
                                {
                                    scDiff = SC_DIFFMAX_LEVELS[levelNum];
                                }
                                else if (scDiff < 10)
                                {
                                    scDiff = 10;
                                }

                                check_time = SDL_GetTicks();
                                showCheck = true;
                                badChain = 0;
                            }
                            else if (scStatus == -1)
                            {
                                // if your attempt was unsuccessful
                                if (chain > hiChain)
                                {
                                    hiChain = chain;
                                }
                                chain = 0;
                                score -= 10;

                                scDiff -= 2;
                                if (scDiff < 10)
                                {
                                    scDiff = 10;
                                }

                                // badChain is to discourage users from spamming the keys until they find the right answer.
                                if (badChain == 0)
                                {
                                    badTime = SDL_GetTicks();
                                }
                                badChain++;
                                if (((SDL_GetTicks() - badTime) < 2000) && (badChain > 3))
                                {
                                    lives--;
                                    dieScreen = true;
                                    dieTime = SDL_GetTicks();
                                    badTime = SDL_GetTicks();
                                    badChain = 0;
                                    Mix_PlayChannel(-1, dieSnd, 0);
                                }
                                else
                                {
                                    Mix_PlayChannel(-1, scBadSound, 0);
                                }
                            }
                        }
                    }
				}

				// the stopClock powerup freezes all the icons for 6 seconds, so you have some extra
				// time to clear the board.
				if (stopClock)
				{
					if ((SDL_GetTicks() - csTime) > 6090)
					{
						stopClock = false;
						Mix_ResumeMusic();
					}
				}
				if ((lives < 1) || ((SDL_GetTicks() - initialTime) > TIME_LEVELS[0][levelNum]))
				{
					break;
				}
				remainingTime = (SDL_GetTicks() - waitTime);
			}

			// all icons drop down by one row. If any icons have made it all the way to the bottom,
			// you lose a life.
			lostLives = dropShortcutIcons();
			if (lostLives > 0)
			{
				lives -= lostLives;
                dieScreen = true;
                dieTime = SDL_GetTicks();
                if (chain > hiChain)
                {
                    hiChain = chain;
                }
                chain = 0;
				scDiff -= 3;
				if (scDiff < 10)
				{
					scDiff = 10;
				}
				if (lives > 0)
				{
					Mix_PlayChannel(-1, dieSnd, 0);
				}
			}

			scGenRate = scDiff / 10;
			addShortcutObject((rand() % scGenRate) + 1); // new icons are added at the top.

			dropNumb++;

			if (SC_DROPTIME[levelNum][0] != SC_DROPTIME[levelNum][1])
			{
				if (dropNumb == HALFTIME_LEVELS[levelNum])
				{
					lvlHalf = 1;
					if (scDiff > SC_DIFFSTART_LEVELS[levelNum])
					{
						scDiff = SC_DIFFSTART_LEVELS[levelNum];
					}

					Mix_PlayMusic(BGmusic1[MUSIC_SPEED[levelNum][1]], -1);
				}
			}

			if (dropNumb == (SC_FINALE_NUMB[levelNum] - 1))
			{
				Mix_PlayMusic(finaleMusic1[MUSIC_SPEED[levelNum][1]], 1);
			}
			else if (dropNumb == SC_FINALE_NUMB[levelNum])
			{
				break;
			}
		}

		if (chain > hiChain)
		{
			hiChain = chain;
		}

		scColumns[0].clear();
		scColumns[1].clear();
		scColumns[2].clear();
		scColumns[3].clear();

		if (lives < 1)
		{
			Mix_PlayMusic(badFinaleMusic1, 1);
		}
		else
		{
			endGameData(hiChain);

            if (event.type == SDL_QUIT)
            {
                clean_up();
                return 0;
            }

			roundPart = 1;
			x = 0;
			y = 0;
			chain = 0;
			hiChain = 0;
			stopClock = false;
			respawn = true;
			csAvail = false;
            dieScreen = false;
            yayScreen = false;
            booScreen = false;
			startFinale = false;
			s = SPD_START_LEVELS[levelNum];
			gr = GR_START_LEVELS[levelNum];

			reviewScreen();

            if (event.type == SDL_QUIT)
            {
                clean_up();
                return 0;
            }

			Mix_PlayMusic(BGmusic2, -1);
			initialTime = SDL_GetTicks();

			thread1 = SDL_CreateThread(gen_thread, "gen_thread", NULL);
		}

		/* This is the "mouse-click modifiers" part of the level. Icons that represent ProTools
		 functions spawn on the right side of the screen and move left.  You must destroy them
		 before they make it to the left side of the screen.  To do this, click on the function icons
		 while pressing the appropriate modifier keys for each function.  (For example, shift-click
		 for the "select contiguous items" icon, etc.) */

		while ((lives > 0) && ((SDL_GetTicks() - initialTime) < TIME_LEVELS[1][levelNum]))
		{
			if (respawn)
			{
				/* The spawning of new function icons is handled a little differently here than in the
				 "keyboard shortcut" rounds. Here, the code tests the boolian "respawn" on every cycle,
				 and spawns a new icon whenever "respawn" is true. Meanwhile, there's a thread called
				 gen_thread() which sets respawn to true at certain intervals.  The amount of time between
				 each icon spawn is random, but it's within a range that is controlled by genRate.  genRate's
				 value is decreased with each correct answer, so the time between icon spawns gets gradually
				 shorter. */

				gRegenTime = SDL_GetTicks();
				respawn = false;

				if (!stopClock)
				{
					vOffset = (rand() % 550) + 1;
					speed = (rand() % 4) + s;
					set = (rand() % SET_LEVELS[levelNum][0]) + SET_LEVELS[levelNum][1];
					keyCombo = (rand() % KC_LEVELS[levelNum][0]) + KC_LEVELS[levelNum][1];

					hk.push_back(Hotkeys(set, keyCombo, vOffset, speed));
				}
			}

			showBG();
			showGameData();
			for (i = 0; i < hk.size(); i++)
			{
				hk[i].ShowTxt();
			}
			SDL_RenderPresent(renderer);

			if (!startFinale)
			{
				if ((TIME_LEVELS[1][levelNum] - (SDL_GetTicks() - initialTime)) <= 4050)
				{
					Mix_PlayMusic(finaleMusic2, 1);
					startFinale = true;
				}
			}

			if (!stopClock)
			{
				for (i = ((hk.size()) - 1); i >= 0; i--)
				{
					if (hk[i].MoveLeft())
					{
						// if you let an icon make it all the way across the screen, you lose a life.
						if (!hk[i].m_Powerup)
						{
							lives--;
							if (lives > 0)
							{
								Mix_PlayChannel(-1, dieSnd, 0);
							}
                            dieScreen = true;
                            dieTime = SDL_GetTicks();
							if (chain > hiChain)
							{
								hiChain = chain;
							}
							chain = 0;
						}
						hk.erase((hk.begin()) + i);
					}

					if (hk.empty())
					{
						break;
					}
				}
			}

			if (lives < 1)
			{
				break;
			}

			if (SDL_PollEvent(&event))
			{
				if ((event.type == SDL_MOUSEBUTTONDOWN) && (!hk.empty()))
				{
					// if you attempt to solve a function icon
					oldScore = score;
					x = event.motion.x;
					y = event.motion.y;
					for (i = ((hk.size()) - 1); i >= 0; i--)
					{
						if (hk[i].CheckKeys(x, y))
						{
							// if the attempt is successful
							if (hk[i].m_Powerup)
							{
								// if the icon has a powerup
								csAvail = true;
							}
							hk.erase((hk.begin()) + i);
                            yayTime = SDL_GetTicks();
                            yayScreen = true;
							increaseDiff();
						}
						if (hk.empty())
						{
							break;
						}
					}
					if (score == oldScore)
					{
						// if the attempt is unsuccessful
						Mix_PlayChannel(-1, failSnd, 0);
						score -= 10;
                        booTime = SDL_GetTicks();
                        booScreen = true;
						if (chain > hiChain)
						{
							hiChain = chain;
						}
						chain = 0;
					}
				}

				else if (event.type == SDL_KEYDOWN)
				{
					if ((event.key.keysym.sym == SDL_SCANCODE_SPACE) && (csAvail) && (!stopClock))
                    {
                        // if you activate a stopClock powerup
                        csAvail = false;
                        csTime = SDL_GetTicks();
                        tmpRemain = ((TIME_LEVELS[1][levelNum] + 999) - (SDL_GetTicks() - initialTime)) / 1000;
                        initialTime += 6100;
                        stopClock = true;
                        Mix_PauseMusic();
                        Mix_PlayChannel(3, csMusic, 0);
                    }

                    else if (event.key.keysym.sym == SDL_SCANCODE_ESCAPE)
                    {
                        // if you pause the game
                        pauseOffset = openPauseMenu();
                        if (pauseOffset == 0)
                        {
                            clean_up();
                            return 0;
                        }
                        initialTime += pauseOffset;
                        csTime += pauseOffset;
                        respawn = false;
                    }
				}

                else if (event.type == SDL_QUIT)
                {
                    clean_up();
                    return 0;
                }
			}

			if (stopClock)
			{
				if ((SDL_GetTicks() - csTime) > 6100)
				{
					stopClock = false;
					Mix_ResumeMusic();
				}
			}
		}

		if (chain > hiChain)
		{
			hiChain = chain;
		}

		if ((lives < 1) && (roundPart == 1))
		{
			Mix_PlayMusic(badFinaleMusic2, 1);
		}

		endGameData(hiChain);

        if (event.type == SDL_QUIT)
        {
            clean_up();
            return 0;
        }

		if (lives < 1)
		{
			levelNum--;
			lives = 5;
			score = 0;
		}

		hk.clear();
	}

    clean_up();

    return 0;
}

// the next 5 functions are for SDL functionality
SDL_Surface *load_image(std::string filename)
{
    SDL_Surface* loadedImage = NULL;
    SDL_Surface* optimizedImage = NULL;

    loadedImage = IMG_Load(filename.c_str());

    if (loadedImage != NULL)
    {
        optimizedImage = SDL_ConvertSurface(loadedImage, screen->format, NULL);
        SDL_FreeSurface(loadedImage);

        if (optimizedImage != NULL)
        {
            SDL_SetColorKey(optimizedImage, SDL_TRUE, SDL_MapRGB( optimizedImage->format, 0, 0xFF, 0xFF));
        }
    }

    return optimizedImage;
}

void apply_surface(int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip)
{
    SDL_Rect offset;

    offset.x = x;
    offset.y = y;

    SDL_BlitSurface(source, clip, destination, &offset);
}

bool init()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        return false;
    }
    
    window = SDL_CreateWindow("HotKeyz", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (window == NULL) {
        cerr << "Unable to open window!\n";
        return false;
    }
    
    renderer = SDL_CreateRenderer(window, -1, 0);

    if (renderer == NULL) {
        cerr << "Unable to open renderer!\n";
        return false;
    }
    
    screen = SDL_GetWindowSurface(window);
    
    if (screen == NULL) {
        cerr << "Unable to open screen!\n";
    }

    if (TTF_Init() == -1)
    {
        cerr << "Unable to init TTF!\n";
        return false;
    }

	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, AUDIO_S16SYS, 2, 4096) == -1)
	{
        cerr << "Unable to init Mix_OpenAudio!\n";
		return false;
	}

	if (Mix_AllocateChannels(4) == -1)
	{
        cerr << "Unable to init Mix_AllocateChannels!\n";
		return false;
	}

    return true;
}

bool load_files()
{
    background[0][0] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/background1_ws.png");
	background[0][1] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/yayBG.png");
	background[0][2] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/booBG.png");
	background[0][3] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/dieBG.png");
	background[1][0] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/inverted.png");
	background[1][1] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/invertedYay.png");
	background[1][2] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/invertedBoo.png");
	background[1][3] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/dieBG.png");
	ptImg[0][0] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key1.png");
	ptImg[0][1] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key2.png");
	ptImg[0][2] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key3.png");
	ptImg[0][3] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key4.png");
	ptImg[0][4] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key5.png");
	ptImg[0][5] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key6.png");
	ptImg[0][6] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key7.png");
	ptImg[0][7] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key8.png");
	ptImg[0][8] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key9.png");
	ptImg[0][9] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key11.png");
	ptImg[0][10] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key14.png");
	ptImg[0][11] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key15.png");
	ptImg[1][0] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key1b.png");
	ptImg[1][1] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key2b.png");
	ptImg[1][2] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key3b.png");
	ptImg[1][3] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key4b.png");
	ptImg[1][4] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key5b.png");
	ptImg[1][5] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key6b.png");
	ptImg[1][6] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key7b.png");
	ptImg[1][7] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key8b.png");
	ptImg[1][8] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key9b.png");
	ptImg[1][9] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key10b.png");
	ptImg[1][10] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key11b.png");
	ptImg[1][11] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/key12b.png");
	csFrame = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/cs_frame.png");
	csPowerup = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/cs_powerup.png");
	review[0] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/review1.png");
	review[1] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/review2.png");
	review[2] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/review3.png");
	review[3] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/review4.png");
	review[4] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/review5.png");
	review[5] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/review6.png");
	review[6] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/review7.png");
	scReview[0] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/scReview1.png");
	scReview[1] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/scReview2.png");
	scReview[2] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/scReview3.png");
	scReview[3] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/scReview4.png");
	scReview[4] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/scReview5.png");
	scReview[5] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/scReview6.png");
	scReview[6] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/scReview7a.png");
	scReview[7] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/scReview7b.png");
	scIcon[0] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/f1.png");
	scIcon[1] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/f2.png");
	scIcon[2] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/f3.png");
	scIcon[3] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/f4.png");
	scIcon[4] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/ctrl_R.png");
	scIcon[5] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/ctrl_T.png");
	scIcon[6] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/return.png");
	scIcon[7] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/opt_return.png");
	scIcon[8] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/ctrl6.png");
	scIcon[9] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/ctrl7.png");
	scIcon[10] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/ctrl_up.png");
	scIcon[11] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/ctrl_down.png");
	scIcon[12] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/opt_openBracket.png");
	scIcon[13] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/opt_closeBracket.png");
	scIcon[14] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/opt_minus.png");
	scIcon[15] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/cmd_equal.png");
	scIcon[16] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/cmdShft_N.png");
	scIcon[17] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/cmd_K.png");
	scIcon[18] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/cmd4.png");
	scIcon[19] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/cmd5.png");
	scIcon[20] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/cmd6.png");
	scIcon[21] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/cmd7.png");
	scIcon[22] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/shft1.png");
	scIcon[23] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/shftS.png");
	scIcon[24] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/cmdG.png");
	scIcon[25] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/optShft_D.png");
	scIcon[26] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/ctrl_tab.png");
	scIcon[27] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/ctrlOpt_tab.png");
	scIcon[28] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/ctrlP.png");
	scIcon[29] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/ctrl_semicolon.png");
	scIcon[30] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/tab.png");
	scIcon[31] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/opt_tab.png");
	scIcon[32] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/opt_KPplus.png");
	scIcon[33] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/opt_KPminus.png");
	scIcon[34] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/ctrlB.png");
	scIcon[35] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/cmdH.png");
	mic[0] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/shotgunMic.png");
	mic[1] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/shotgunMicArrows.png");
	scBackground[0] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/scBackground.png");
	scBackground[1] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/scBackground_die.png");
	scBackground[2] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/scBackground_cs.png");
	warning = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/warning.png");
	wrongAns = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/ban.png");
	rightAns = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/checkmark.png");
	scClockStop = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/scClockStop.png");
	wave[0][0] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/wave1a.png");
	wave[0][1] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/wave1b.png");
	wave[0][2] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/wave1c.png");
	wave[0][3] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/wave1d.png");
	wave[1][0] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/wave2a.png");
	wave[1][1] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/wave2b.png");
	wave[1][2] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/wave2c.png");
	wave[1][3] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/wave2d.png");
	wave[2][0] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/wave3a.png");
	wave[2][1] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/wave3b.png");
	wave[2][2] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/wave3c.png");
	wave[2][3] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/wave3d.png");
	wave[3][0] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/wave4a.png");
	wave[3][1] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/wave4b.png");
	wave[3][2] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/wave4c.png");
	wave[3][3] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/wave4d.png");
	wave[4][0] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/wave5a.png");
	wave[4][1] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/wave5b.png");
	wave[4][2] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/wave5c.png");
	wave[4][3] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/wave5d.png");
	keyboardPrompt = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/keyboard_prompt.png");
	keyboardTypes[0] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/fullSized.png");
	keyboardTypes[1] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/laptop.png");
	endScreen = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/endScreen.png");
    pauseMenu[0] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/pause1.png");
    pauseMenu[1] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/pause2.png");

    font = TTF_OpenFont("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/Arial Bold.ttf", 32);
	smallFont = TTF_OpenFont("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/Arial.ttf", 26);
	bigFont = TTF_OpenFont("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/Arial Bold.ttf", 50);

	failSnd = Mix_LoadWAV("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/failB.wav");
	dieSnd = Mix_LoadWAV("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/deathSound.wav");
	succeedSnd = Mix_LoadWAV("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/succeed2.wav");
	csMusic = Mix_LoadWAV("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/csMusicA.wav");
	scStartSound[0] = Mix_LoadWAV("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/startSound.wav");
	scStartSound[1] = Mix_LoadWAV("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/startSound2.wav");
	scGoodSound = Mix_LoadWAV("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/scGoodSound.wav");
	scBadSound = Mix_LoadWAV("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/scBadSound.wav");
	oneUpSnd2 = Mix_LoadWAV("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/oneUp2a.wav");
	oneUpSnd1 = Mix_LoadWAV("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/oneUp1.wav");

	BGmusic2 = Mix_LoadMUS("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/BGmusic2a.wav");
	BGmusic1[0] = Mix_LoadMUS("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/BGmusic1.wav");
	BGmusic1[1] = Mix_LoadMUS("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/BGmusic1_faster.wav");
	BGmusic1[2] = Mix_LoadMUS("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/BGmusic1_fastest.wav");
	finaleMusic1[0] = Mix_LoadMUS("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/finale1b.wav");
	finaleMusic1[1] = Mix_LoadMUS("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/finale1_faster.wav");
	finaleMusic1[2] = Mix_LoadMUS("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/finaleMusic1_fastest.wav");
	finaleMusic2 = Mix_LoadMUS("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/finale2.wav");
	reviewMusic2 = Mix_LoadMUS("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/reviewMusic2.wav");
	reviewMusic1 = Mix_LoadMUS("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/reviewMusic1.wav");
	badFinaleMusic1 = Mix_LoadMUS("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/badFinale1.wav");
	badFinaleMusic2 = Mix_LoadMUS("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/badFinale2.wav");

    if ((background[0][0] == NULL) || (background[1][0] == NULL) || (ptImg[0][0] == NULL) || (ptImg[1][0] == NULL) || (csFrame == NULL) || (csPowerup == NULL))
    {
        return false;
    }

	if ((scIcon[0] == NULL) || (mic[0] == NULL) || (scBackground[0] == NULL) || (warning == NULL) || (wrongAns == NULL) || (rightAns == NULL) || (scClockStop == NULL))
	{
		return false;
	}
	if ((wave[0][0] == NULL) || (wave[1][0] == NULL) || (wave[2][0] == NULL) || (wave[3][0] == NULL) || (wave[4][0] == NULL) || (scIcon[8] == NULL))
	{
		return false;
	}

	if ((scIcon[18] == NULL) || (scIcon[26] == NULL) || (scIcon[34] == NULL) || (keyboardPrompt == NULL) || (keyboardTypes[0] == NULL) || (endScreen == NULL))
	{
		return false;
	}

    if ((font == NULL) || (bigFont == NULL))
    {
        return false;
    }

	if ((succeedSnd == NULL) || (dieSnd == NULL) || (failSnd == NULL) || (BGmusic2 == NULL) || (BGmusic1[0] == NULL) || (scStartSound == NULL) || (finaleMusic1[0] == NULL))
	{
		return false;
	}

	if ((finaleMusic2 == NULL) || (scGoodSound == NULL) || (scBadSound == NULL) || (oneUpSnd2 == NULL)|| (oneUpSnd1 == NULL) || (reviewMusic2 == NULL))
	{
		return false;
	}

	if ((reviewMusic1 == NULL) || (badFinaleMusic1 == NULL) || (badFinaleMusic2 == NULL))
	{
		return false;
	}

	chooseTxt = TTF_RenderText_Solid(font, "Choose a level (1-7)", textColor);

    return true;
}

void clean_up()
{
    shutdownThread.store(true);
    int thread_return_status = 0;
    SDL_WaitThread(thread1, &thread_return_status);

	SDL_FreeSurface(csFrame);
	SDL_FreeSurface(csPowerup);
	SDL_FreeSurface(mic[0]);
	SDL_FreeSurface(mic[1]);
	SDL_FreeSurface(scBackground[0]);
	SDL_FreeSurface(scBackground[1]);
	SDL_FreeSurface(scBackground[2]);
	SDL_FreeSurface(warning);
	SDL_FreeSurface(wrongAns);
	SDL_FreeSurface(rightAns);
	SDL_FreeSurface(scClockStop);
	SDL_FreeSurface(keyboardTypes[0]);
	SDL_FreeSurface(keyboardTypes[1]);
    SDL_FreeSurface(pauseMenu[0]);
    SDL_FreeSurface(pauseMenu[1]);
	SDL_FreeSurface(keyboardPrompt);
	SDL_FreeSurface(endScreen);

	short int i, i2;

	for (i = 0; i < 4; i++)
	{
		SDL_FreeSurface(background[0][i]);
		SDL_FreeSurface(background[1][i]);
	}

	for (i = 0; i < 12; i++)
	{
		SDL_FreeSurface(ptImg[0][i]);
		SDL_FreeSurface(ptImg[1][i]);
	}

	for (i = 0; i < 7; i++)
	{
		SDL_FreeSurface(review[i]);
		SDL_FreeSurface(scReview[i]);
	}

	SDL_FreeSurface(scReview[7]);

	for (i = 0; i < 36; i++)
	{
		SDL_FreeSurface(scIcon[i]);
	}

	for (i = 0; i < 5; i++)
	{
		for (i2 = 0; i2 < 4; i2++)
		{
			SDL_FreeSurface(wave[i][i2]);
		}
	}

    TTF_CloseFont(font);
    TTF_CloseFont(smallFont);
	TTF_CloseFont(bigFont);

	Mix_AllocateChannels(0);
	Mix_FreeChunk(failSnd);
	Mix_FreeChunk(dieSnd);
	Mix_FreeChunk(succeedSnd);
	Mix_FreeChunk(csMusic);
	Mix_FreeChunk(scStartSound[0]);
	Mix_FreeChunk(scStartSound[1]);
	Mix_FreeChunk(scGoodSound);
	Mix_FreeChunk(scBadSound);
	Mix_FreeChunk(oneUpSnd2);
	Mix_FreeChunk(oneUpSnd1);
	Mix_FreeMusic(BGmusic2);
	Mix_FreeMusic(BGmusic1[0]);
	Mix_FreeMusic(BGmusic1[1]);
	Mix_FreeMusic(BGmusic1[2]);
	Mix_FreeMusic(finaleMusic1[0]);
	Mix_FreeMusic(finaleMusic1[1]);
	Mix_FreeMusic(finaleMusic1[2]);
	Mix_FreeMusic(finaleMusic2);
	Mix_FreeMusic(reviewMusic2);
	Mix_FreeMusic(reviewMusic1);
	Mix_FreeMusic(badFinaleMusic1);
	Mix_FreeMusic(badFinaleMusic2);
	Mix_CloseAudio();

    TTF_Quit();

    SDL_Quit();
}

// eliminates any key combos that involve the numeric keypad, if you opt to use a laptop keyboard.
bool getKeyboardType()
{
	short int x = 0, y = 0, kbType = 0;

	while (true)
	{
		apply_surface(0, 0, keyboardPrompt, screen);
		apply_surface(0, 0, keyboardTypes[kbType], screen);
		SDL_RenderPresent(renderer);

		while (!SDL_PollEvent(&event))
		{}

		if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			x = event.motion.x;
			y = event.motion.y;
			if ((x > 307) && (x < 1044) && (y > 163) && (y < 312))
			{
				kbType = 0;
			}
			else if ((x > 307) && (x < 1044) && (y > 331) && (y < 479))
			{
				kbType = 1;
			}
			else if ((x > 480) && (x < 870) && (y > 512) && (y < 602))
			{
				break;
			}
		}
		else if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.sym == SDL_SCANCODE_UP)
			{
				kbType = 0;
			}
			else if (event.key.keysym.sym == SDL_SCANCODE_DOWN)
			{
				kbType = 1;
			}
			else if ((event.key.keysym.sym == SDL_SCANCODE_RETURN) || (event.key.keysym.sym == SDL_SCANCODE_KP_ENTER))
			{
				break;
			}
		}
        else if (event.type == SDL_QUIT)
        {
            break;
        }
	}

	if (kbType == 1)
	{
		scReview[4] = NULL;
		scReview[5] = NULL;
		scReview[7] = NULL;
		scIcon[32] = NULL;
		scIcon[33] = NULL;

		scReview[4] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/scReview5b.png");
		scReview[5] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/scReview6b.png");
		scReview[7] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/scReview7bB.png");
		scIcon[32] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/ctrl_period.png");
		scIcon[33] = load_image("/Users/jbell/Documents/KeyPolling + Threads/build/Debug/ctrl_comma.png");

		shortcut_codes[0][32] = 1;
		shortcut_codes[1][32] = 46;
		shortcut_codes[0][33] = 1;
		shortcut_codes[1][33] = 44;
	}

	if ((scReview[4] == NULL) || (scReview[5] == NULL) || (scReview[7] == NULL) || (scIcon[32] == NULL) || (scIcon[33] == NULL))
	{
		return false;
	}

	return true;
}

// shows basic text info during gameplay
void showGameData()
{
	unsigned int remain;

	if (stopClock)
	{
		remain = tmpRemain;
	}
	else
	{
		remain = ((TIME_LEVELS[roundPart][levelNum] + 999) - (SDL_GetTicks() - initialTime)) / 1000;
	}

	sScore.str("");
	sLives.str("");
	sChain.str("");
	sRemaining.str("");

	sScore << "Score: " << score;
	sLives << "Lives: " << lives;
	sChain << "Chain: " << chain;
	sRemaining << "Time left: " << remain;

	scoreTxt = TTF_RenderText_Solid(smallFont, sScore.str().c_str(), textColor);
	livesTxt = TTF_RenderText_Solid(smallFont, sLives.str().c_str(), textColor);
	chainTxt = TTF_RenderText_Solid(smallFont, sChain.str().c_str(), textColor);
	timeTxt = TTF_RenderText_Solid(smallFont, sRemaining.str().c_str(), textColor);

	apply_surface(10, 3, timeTxt, screen);
	apply_surface(10, 28, scoreTxt, screen);
	apply_surface(10, 53, livesTxt, screen);
	apply_surface(10, 78, chainTxt, screen);

	if (csAvail)
	{
		apply_surface(160, -10, csPowerup, screen);
	}
	if (showCheck)
	{
        apply_surface(X_ICON[check_position[0]], Y_ICON[check_position[1]], wave[check_position[1]][(SDL_GetTicks() / 50) % 4], screen);
		apply_surface(X_ICON[check_position[0]], Y_ICON[check_position[1]], rightAns, screen);
		if ((SDL_GetTicks() - check_time) > 300)
		{
			showCheck = false;
		}
	}
}

// shows info at the end of a round
void endGameData(unsigned short int bc)
{
	sScore.str("");
	sChain.str("");
	sLvlEndTxt1.str("");
	sLvlEndTxt2.str("");

	sScore << "Score: " << score;
	sChain << "Best chain: " << bc;

	if (lives < 1)
	{
		sLvlEndTxt1 << "    Level " << levelNum + 1 << " failed";
		sLvlEndTxt2 << "                          Press [space] to try Level " << levelNum + 1 << " again";
	}
	else
	{
		if (roundPart == 0)
		{
			sLvlEndTxt1 << "        Success!";
			sLvlEndTxt2 << "Press [space] to continue on to the mouse-click modifier round";
		}
		else
		{
			sLvlEndTxt1 << "  Level " << levelNum + 1 << " complete!";
			sLvlEndTxt2 << "                        Press [space] to continue on to Level " << levelNum + 2;
		}
	}

	finalTxt = TTF_RenderText_Solid(bigFont, sLvlEndTxt1.str().c_str(), textColor);
	scoreTxt = TTF_RenderText_Solid(font, sScore.str().c_str(), textColor);
	chainTxt = TTF_RenderText_Solid(font, sChain.str().c_str(), textColor);
	continueTxt = TTF_RenderText_Solid(font, sLvlEndTxt2.str().c_str(), textColor);

	if (roundPart == 0)
	{
		apply_surface(0, 0, scBackground[0], screen);
	}
	else
	{
		apply_surface(0, 0, background[0][0], screen);
	}

	apply_surface(440, 300, finalTxt, screen);
	apply_surface(550, 355, chainTxt, screen);
	apply_surface(565, 385, scoreTxt, screen);
	apply_surface(160, 500, continueTxt, screen);

	if ((levelNum == 6) && (roundPart == 1) && (lives > 0))
	{
		apply_surface(0, 0, endScreen, screen);
		apply_surface(120, 410, chainTxt, screen);
		apply_surface(120, 440, scoreTxt, screen);
	}

	SDL_RenderPresent(renderer);

	SDL_EventState(SDL_KEYDOWN, SDL_IGNORE);
	SDL_Delay(250);
	SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);

	while (true)
	{
		if (SDL_PollEvent(&event))
		{
			if ((event.key.keysym.sym == SDL_SCANCODE_SPACE) || (event.type == SDL_QUIT))
            {
                break;
            }
		}
	}
}

// shows the background image during the mouse-click modifier round.
void showBG()
{
	short int bgCode1 = 0;
	short int bgCode2 = 0;

	if (stopClock)
	{
		bgCode1 = 1;
	}

    if (yayScreen)
    {
        if ((SDL_GetTicks() - yayTime) > 500)
        {
            yayScreen = false;
        }
        else
        {
            bgCode2 = 1;
        }
    }

    if (booScreen)
    {
        if ((SDL_GetTicks() - booTime) > 500)
        {
            booScreen = false;
        }
        else
        {
            bgCode2 = 2;
        }
    }

    if (dieScreen)
    {
        if ((SDL_GetTicks() - dieTime) > 500)
        {
            dieScreen = false;
        }
        else
        {
            bgCode2 = 3;
        }
    }

	apply_surface(0, 0, background[bgCode1][bgCode2], screen);
}

// shows the background image during the keyboard shortcut round.
void showShortcutBG()
{
	short int bgCode = 0;

    if (dieScreen)
    {
        if ((SDL_GetTicks() - dieTime) > 500)
        {
            dieScreen = false;
        }
        else
        {
            bgCode = 1;
        }
    }

	if (stopClock)
	{
		bgCode = 2;
		dieScreen = false;
	}

	apply_surface(0, 0, scBackground[bgCode], screen);
}

// shows the solutions for all the current keyboard shortcuts at the beginning of each keyboard shortcut round.
void scReviewScreen()
{
	bool keepLooping = true;
	short int reviewNum = 6;

	Mix_PlayMusic(reviewMusic1, -1);

	apply_surface(0, 0, scBackground[0], screen);
	apply_surface(0, 0, scReview[levelNum], screen);
	SDL_RenderPresent(renderer);

	SDL_EventState(SDL_KEYDOWN, SDL_IGNORE);
	SDL_Delay(500);
	SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);

	if (levelNum < 6)
	{
		while (true)
		{
			if (SDL_PollEvent(&event))
			{
				if ((event.key.keysym.sym == SDL_SCANCODE_SPACE) || (event.type == SDL_QUIT))
                {
                    break;
                }
			}
		}
	}
	else
	{
		while (keepLooping)
		{
			apply_surface(0, 0, scBackground[0], screen);
			apply_surface(0, 0, scReview[reviewNum], screen);
			SDL_RenderPresent(renderer);

			while (true)
			{
				if (SDL_PollEvent(&event))
				{
                    if (event.type == SDL_KEYDOWN)
                    {
                        if (event.key.keysym.sym == SDL_SCANCODE_LEFT)
                        {
                            reviewNum--;
                            if (reviewNum < 6)
                            {
                                reviewNum = 6;
                            }
                            break;
                        }
                        if (event.key.keysym.sym == SDL_SCANCODE_RIGHT)
                        {
                            reviewNum++;
                            if (reviewNum > 7)
                            {
                                reviewNum = 7;
                            }
                            break;
                        }
                        if (event.key.keysym.sym == SDL_SCANCODE_SPACE)
                        {
                            keepLooping = false;
                            break;
                        }
                    }
				}
                else if (event.type == SDL_QUIT)
                {
                    break;
                }
			}
		}
	}

	Mix_HaltMusic();
}

// reviews the solutions for all the current mouse-click commands at the beginning of each mouse-click modifier round.
void reviewScreen()
{
	apply_surface(0, 0, review[levelNum], screen);
	SDL_RenderPresent(renderer);

	Mix_PlayMusic(reviewMusic2, -1);

	SDL_EventState(SDL_KEYDOWN, SDL_IGNORE);
	SDL_Delay(500);
	SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);

	while (true)
	{
		if (SDL_PollEvent(&event))
		{
			if ((event.key.keysym.sym == SDL_SCANCODE_SPACE) || (event.type == SDL_QUIT))
            {
                break;
            }
		}
	}
}

// this is a debug cheat
short int getLevel()
{
	short int usr = -1;
	while (usr == -1)
	{
        if(SDL_PollEvent(&event))
        {
            if(event.type == SDL_KEYDOWN)
            {
				usr = event.key.keysym.sym;
				if ((usr > 256) && (usr < 264))
				{
					usr -= 256;
				}
				else if ((usr > 48) && (usr < 56))
				{
					usr -= 48;
				}
				else
				{
					usr = -1;
				}
			}
        }
	}
	return usr - 1;
}

// increases difficulty. is called every time you get a correct answer in the mouse-click modifier round.
void increaseDiff()
{
	if (chain < 9)
	{
		score += (10 + (chain * 5));
	}
	else
	{
		score += 50;
	}
	chain++;

	if ((gr > GR_CAP_LEVELS[levelNum]) && (!stopClock))
	{
		gr -= GR_LEVELS[levelNum];
	}

	if ((s < 6) && (!stopClock))
	{
		s += SPEED_LEVELS[levelNum];
	}

	if ((score >= (scLog * 1000)) && (oldScore < (scLog * 1000)))
	{
		lives++;
		scLog++;
		Mix_PlayChannel(-1, oneUpSnd2, 0);
	}
	else
	{
		Mix_PlayChannel(-1, succeedSnd, 0);
	}

}

// called every time you get a correct answer in the keyboard shortcut round.
void scScore()
{
	if (chain < 9)
	{
		score += (10 + (chain * 5));
	}
	else
	{
		score += 50;
	}
	chain++;

	if ((score >= (scLog * 1000)) && (oldScore < (scLog * 1000)))
	{
		lives++;
		scLog++;
		Mix_PlayChannel(-1, oneUpSnd1, 0);
	}
	else
	{
		Mix_PlayChannel(-1, scGoodSound, 0);
	}

}

// adds at least one icon to the top of the pile in the keyboard shortcuts round.
void addShortcutObject(short int reps)
{
	short int i, col, scNum;

	for (i = 0; i < reps; i++)
	{
		col = rand() % 4;
		scNum = (rand() % SC_LEVELS[levelNum][0]) + SC_LEVELS[levelNum][1];

		if (scColumns[col].empty())
		{
			scColumns[col].push_back(Shortcuts(scNum, shortcut_codes[0][scNum], shortcut_codes[1][scNum], col));
		}
		else
		{
			if (scColumns[col][(scColumns[col].size()) - 1].m_Row == 0)
			{
				i--;
			}
			else
			{
				scColumns[col].push_back(Shortcuts(scNum, shortcut_codes[0][scNum], shortcut_codes[1][scNum], col));
			}
		}
	}
}

// shows all the icons in the keyboard shortcuts round.
void showShortcutIcons()
{
	short int x, i;

	for (x = 0; x < 4; x++)
	{
		for (i = ((scColumns[x].size()) - 1); i >= 0; i--)
		{
			scColumns[x][i].ShowIcon();
		}
	}
}

// drops all the icons down by one row in the keyboard shortcuts round.
short int dropShortcutIcons()
{
	short int x, i;
	short int lostLives = 0;

	for (x = 0; x < 4; x++)
	{
		for (i = ((scColumns[x].size()) - 1); i >= 0; i--)
		{
			if (scColumns[x][i].MoveDown())
			{
				lostLives++;
				scColumns[x].erase((scColumns[x].begin()) + i);
			}
			if (scColumns[x].empty())
			{
				break;
			}
			else
			{
				if ((rand() % SHORTCUT_CS_CHANCE[levelNum]) == 1)
				{
					if (!csLocked)
					{
						cs_position[0] = x;
						cs_position[1] = scColumns[x][rand() % (scColumns[x].size())].m_Row;
						csLocked = true;
						scClockStop_time = SDL_GetTicks();
					}
				}
			}
		}
	}

	return lostLives;
}

// checks to see if an attempted answer is correct in the keyboard shortcuts round.
short int checkShortcut(short int column)
{
	short int status = 0;

	if (!scColumns[column].empty())
	{
		if (scColumns[column][0].CheckInput())
		{
			check_position[0] = column;
			check_position[1] = scColumns[column][0].m_Row;
			if (csLocked)
			{
				if ((cs_position[0] == column) && (cs_position[1] == scColumns[column][0].m_Row))
				{
					csLocked = false;
					csAvail = true;
				}
			}
			switch (scColumns[column][0].m_Row)
			{
				case 0:
					status = 4;
					break;
				case 1:
					status = 3;
					break;
				case 2:
					status = 3;
					break;
				case 3:
					status = 2;
					break;
				case 4:
					status = 1;
					break;
				default:
					break;
			}
			scColumns[column].erase(scColumns[column].begin());
		}
		else
		{
			status = -1;
		}
	}

	return status;
}

// moves the microphone icon whenever the user presses left or right
void moveMic(short int& rMicPos)
{
	switch (event.key.keysym.sym)
	{
		case SDL_SCANCODE_LEFT:
			rMicPos--;
			if (rMicPos < 0)
			{
				rMicPos = 3;
			}
			break;
		case SDL_SCANCODE_RIGHT:
			rMicPos++;
			if (rMicPos > 3)
			{
				rMicPos = 0;
			}
			break;
		default:
			break;
	}
}

// shows the clock-stopper powerup on top of one of the icons, in the keyboard shortcuts round.
void showSCclockstop()
{
	if (csLocked)
	{
		if (((SDL_GetTicks() / 200) % 2) == 1)
		{
			apply_surface(X_ICON[cs_position[0]], Y_ICON[cs_position[1]], scClockStop, screen);
		}
		if ((SDL_GetTicks() - scClockStop_time) > (SC_DROPTIME[levelNum][lvlHalf] - 20))
		{
			csLocked = false;
		}
	}
}

// pause menu
Uint32 openPauseMenu()
{
    Uint32 current = SDL_GetTicks();
    short int x, y, menuType = 0;

    if (stopClock)
    {
        Mix_Pause(3);
    }
    else
    {
        Mix_PauseMusic();
    }

    SDL_EventState(SDL_KEYDOWN, SDL_IGNORE);
    SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
	SDL_Delay(250);
	SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);
    SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_ENABLE);

    while (true)
    {
        apply_surface(0, 0, pauseMenu[menuType], screen);
        SDL_RenderPresent(renderer);

        while (!SDL_PollEvent(&event))
        {}

        if (event.type == SDL_KEYDOWN)
        {
            if ((event.key.keysym.sym == SDL_SCANCODE_RETURN) || (event.key.keysym.sym == SDL_SCANCODE_KP_ENTER))
            {
                break;
            }
            else if (event.key.keysym.sym == SDL_SCANCODE_UP)
            {
                menuType = 0;
            }
            else if (event.key.keysym.sym == SDL_SCANCODE_DOWN)
            {
                menuType = 1;
            }
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            x = event.motion.x;
			y = event.motion.y;

            if ((x > 372) && (x < 920) && (y > 272) && (y < 379))
            {
                menuType = 0;
                break;
            }

            if ((x > 372) && (x < 920) && (y > 416) && (y < 523))
            {
                menuType = 1;
                break;
            }
        }
        else if (event.type == SDL_QUIT)
        {
            menuType = 1;
            break;
        }
    }

    if (menuType == 1)
    {
        return 0;
    }

    if (stopClock)
    {
        Mix_Resume(3);
    }
    else
    {
        Mix_ResumeMusic();
    }
    return (SDL_GetTicks() - current);
}

// this thread controls when new icons appear in the mouse-click modifier round.
int gen_thread(void *data)
{
	Uint32 genRate;
	Uint32 regenTime;

	SDL_Delay(50);
    
    // Check if we've been asked to shut down.
    if (shutdownThread.load()) {
        return 0;
    }

	while (roundPart == 1)
	{
		genRate = (rand() % 750) + gr;
		regenTime = gRegenTime;

		while ((SDL_GetTicks() - regenTime) < genRate)
		{
			if (hk.empty())
			{
				break;
			}
            
            // Check if we've been asked to shut down.
            if (shutdownThread.load()) {
                break;
            }
		}

		respawn = true;
		SDL_Delay(50);
        
        // Check if we've been asked to shut down.
        if (shutdownThread.load()) {
            return 0;
        }
	}

	return 0;
}
