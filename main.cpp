#include <windows.h>
#include <bits/stdc++.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

using namespace std;

const int SCREEN_WIDTH = 1400;
const int SCREEN_HEIGHT = 600;
const int BUTTON_TYPE1_WIDTH = 200;
const int BUTTON_TYPE1_HEIGHT = 100;
const int BUTTON_TYPE2_WIDTH = 200;
const int BUTTON_TYPE2_HEIGHT = 100;

const string GAME_NAME = "AKALI VS DRAGON";

enum ButtonSprite
{
	BUTTON_MOUSE_OUT = 0,
	BUTTON_MOUSE_OVER = 1,
	BUTTON_TOTAL = 2
};

struct Circle
{
	int x, y;
	int r;
};

void logSDLError(std::ostream& os,
                 const std::string &msg, bool fatal)
{
    os << msg << " Error: " << SDL_GetError() << std::endl;
    if (fatal) {
        SDL_Quit();
        exit(1);
    }
}

void initSDL(SDL_Window* &window, SDL_Renderer* &renderer)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        logSDLError(std::cout, "SDL_Init", true);

    window = SDL_CreateWindow(GAME_NAME.c_str(), SDL_WINDOWPOS_CENTERED,
       SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
//    window = SDL_CreateWindow(WINDOW_TITLE.c_str(), SDL_WINDOWPOS_CENTERED,
//       SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (window == nullptr) logSDLError(std::cout, "CreateWindow", true);


    //Khi chạy trong môi trường bình thường (không chạy trong máy ảo)
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED |
                                              SDL_RENDERER_PRESENTVSYNC);
    //Khi chạy ở máy ảo (ví dụ tại máy tính trong phòng thực hành ở trường)
    //renderer = SDL_CreateSoftwareRenderer(SDL_GetWindowSurface(window));

    if (renderer == nullptr) logSDLError(std::cout, "CreateRenderer", true);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void waitUntilKeyPressed()
{
    SDL_Event e;
    while (true) {
        if ( SDL_WaitEvent(&e) != 0 &&
             (e.key.keysym.sym == SDLK_ESCAPE || e.type == SDL_QUIT) )
            return;
        SDL_Delay(100);
    }
}

void quitSDL(SDL_Window* window, SDL_Renderer* renderer)
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* gFont = NULL;
Mix_Music* GMusic = NULL;

class texture{
    SDL_Texture* ttx;

    int width;
    int height;
    int posX;
    int posY;
    int HP_WIDTH;
    int HP_HEIGHT;

public:

    texture()
    {
        ttx = NULL;
        width = 0;
        height = 0;
        posX = 0;
        posY = 0;
        HP_WIDTH = 0;
        HP_HEIGHT = 0;
    }

    void loadFromFile(string path)
    {
        SDL_Texture* tx;
        SDL_Surface* sf = IMG_Load(path.c_str());
        if( sf == NULL )
        {
            printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
        }
        else
        {
            //Color key image
            SDL_SetColorKey( sf, SDL_TRUE, SDL_MapRGB( sf->format, 0, 0xFF, 0xFF ) );
            //Create texture from surface pixels
            tx = SDL_CreateTextureFromSurface(renderer, sf);
            if( tx == NULL )
            {
                printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
            }
            else
            {
                //Get image dimensions
                width = sf->w;
                height = sf->h;
            }
            //Get rid of old loaded surface
            SDL_FreeSurface(sf);
        }
        //Return success
        ttx = tx;
    }

    bool loadFromRenderedText( string textureText, SDL_Color textColor )
    {
        //Get rid of preexisting texture
        free();

        //Render text surface
        SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
        if( textSurface == NULL )
        {
            printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
        }
        else
        {
            //Create texture from surface pixels
            ttx = SDL_CreateTextureFromSurface( renderer, textSurface );
            if( ttx == NULL )
            {
                printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
            }
            else
            {
                //Get image dimensions
                width = textSurface->w;
                height = textSurface->h;
            }

            //Get rid of old surface
            SDL_FreeSurface( textSurface );
        }

        //Return success
        return ttx != NULL;
    }

    texture(string path)
    {
        loadFromFile(path);
        posX = 0;
        posY = 0;
    }

    void free()
    {
        if(ttx != NULL)
        {
            SDL_DestroyTexture(ttx);
            ttx = NULL;
            width = 0;
            height = 0;
            posX = 0;
            posY = 0;
        }
    }

    void setW(int W)
    {
        width = W;
    }

    void setH(int H)
    {
        height = H;
    }

    void render(int x, int y, SDL_Renderer* gRenderer, SDL_Rect* clip = nullptr, double angle = 0.0,
					SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE)
    {
        SDL_Rect renderSpace;
        renderSpace = { x, y, width, height};
        if (clip != nullptr)
        {
            renderSpace.w = clip->w;
            renderSpace.h = clip->h;
        }
        SDL_RenderCopyEx(renderer, ttx, clip, &renderSpace, angle, center, flip);
    }

    int getWidth()
    {
        return HP_WIDTH;
    }
    int getHeight()
    {
        return HP_HEIGHT;
    }
    int getX()
    {
        return posX;
    }
    int getY()
    {
        return posY;
    }
    void setlocate(int X,int Y)
    {
        posX = X;
        posY = Y;
    }
};

class Character
{
    int MAX_HP, HP, DAME;
    int HP_posX, HP_posY;
    int HP_BG_posX, HP_BG_posY;

    int posX, posY;
	int velX, velY;

	int CD_attack1;
	int CD_attack2;

	bool hited;
	bool alive;

	texture HP_BAR;
	texture HP_BAR_BG;

	int DF_HP_BARH;

	SDL_Rect Collider;

	static const int CHARACTER_VEL = 10;
public:

    string type;

	Character(int X_,int Y_, int MAX_HP_ = 100, int DAME_ = 10)
	{
	    HP_posX = HP_posY = 0;
	    HP = MAX_HP_;
	    DAME = DAME_;
	    posX = X_;
        posY = Y_;
        CD_attack2 = 0;

        velX = 0;
        velY = 0;

        hited = false;
        alive = true;

        Collider.x = posX;
        Collider.y = posY;

        HP_BAR.loadFromFile("img/HEALTH_BAR.png");
        HP_BAR_BG.loadFromFile("img/HEALTH_BAR_BG.png");
        HP_BAR.setW(HP);
        DF_HP_BARH = 20;
        HP_BAR.setH(DF_HP_BARH);
	}

	void HandleEvent(SDL_Event& e, bool& UpDown, bool& LeftRight, bool& srk)
	{
        if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
        {
            switch( e.key.keysym.sym )
            {
                case SDLK_a: velX = -CHARACTER_VEL; LeftRight = true; break;
                case SDLK_d: velX = CHARACTER_VEL; LeftRight = false; break;
            }
        }
        else if( e.type == SDL_KEYUP && e.key.repeat == 0)
        {
            switch( e.key.keysym.sym )
            {
                case SDLK_a:
                if (LeftRight)
                {
                    velX = 0;
                }
                break;
                case SDLK_d:
                if (!LeftRight)
                {
                    velX = 0;
                }
                break;
            }
        }
        if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
        {
            if (e.key.keysym.sym == SDLK_w && velY != CHARACTER_VEL)
                velY = -CHARACTER_VEL;
        }

        if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
        {
            switch(e.key.keysym.sym)
            {
                case SDLK_k:
                    if (CD_attack2 == 0)
                        {
                            srk = true;
                            CD_attack2 = 200;
                        }
                    break;
            }
        }
	}

	void change_HP_DF(int H)
	{
	    DF_HP_BARH = H;
	}

	bool live()
	{
	    return alive;
	}

	void setCollider(int w,int h)
	{
	    Collider.w = w;
	    Collider.h = h;
	}

	void CDR()
	{
        if (CD_attack2 > 0)
            CD_attack2 -= 10;
	}

	void SetVel(int x, int y)
	{
	    velX = x;
	    velY = y;
	}

	void HPSETUP(int w,int h,int px ,int py)
	{
	    HP_BAR_BG.setW(w);
	    HP_BAR_BG.setH(h);
	    HP_BG_posX = px;
	    HP_BG_posY = py;
	    HP_BAR.setW(w);
	    HP_BAR.setH(h);
	    HP_posX = px;
	    HP_posY = py;

	}

	void Move()
	{
        posX += velX;

        if( ( posX < -10 ) || ( posX > SCREEN_WIDTH - 60 ) )
        {
            posX -= velX;
        }
        Collider.x = posX;
	}

	void UP()
	{
        if (velY == 0)
            return;
        if (posY > 260)
            posY += velY;
        if (posY == 260)
        {
            velY = CHARACTER_VEL;
            posY += velY;
        }
        if (posY == 420)
            velY = 0;
        Collider.y = posY;
	}

	void FALL(int X)
	{
	    if (posY < X)
            posY += velY;
        Collider.y = posY;
	}


	int GetHit()
	{
	    return hited;
	}

	void HIT()
	{
	    hited = true;
	}

	void Render(SDL_Rect* currentClip, SDL_Renderer *renderer, texture CharTx, double angle = 0.0)
	{
	    CharTx.render(posX, posY, renderer, currentClip, angle);
	}

	void Render_HP(SDL_Rect* Clip, SDL_Renderer* renderer)
	{
	    HP_BAR_BG.render(HP_BG_posX, HP_BG_posY, renderer);
	    HP_BAR.render(HP_posX, HP_posY , renderer, Clip);
	}

	int GetPosX()
	{
	    return posX;
	}

	int GetPosY()
	{
	    return posY;
	}

	void SET_HP(int X)
	{
	    HP = X;
	    HP_BAR.setW(X);
	    HP_BAR.setH(DF_HP_BARH);
	}

	int GET_HEALTH()
	{
	    return HP;
	}

	void set_HP_locate(int X,int Y)
	{
	    HP_posX = X;
	    HP_posY = Y;
	}

	void REDUCE_HP(int X)
	{
	    HP -= X;
        HP_BAR.setW(HP);
        HP_BAR.setH(DF_HP_BARH);
	    if (HP <= 0)
        {
            alive = false;
            HP = 0;
        }
	}

	void SET_DAME(int X)
	{
	    DAME = X;
	}

	int DAMAGE()
	{
	    return DAME;
	}

	SDL_Rect GetRect()
	{
	    return Collider;
	}
};

texture renderFromText(string text, string font, int sizef)
{
    texture res;
    if( TTF_Init() == -1 )
        printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );

    gFont = TTF_OpenFont(font.c_str(), sizef);
    if( gFont == NULL )
        printf( "Failed to load font! SDL_ttf Error: %s\n", TTF_GetError() );
    else
    {
        SDL_Color textColor = { 0, 0, 0 };
        if( !res.loadFromRenderedText( text, textColor ) )
            printf( "Failed to render text texture!\n" );
    }
    return res;
}

class Button
{
public:

	Button()
	{
	    position.x = 0;
        position.y = 0;
	}

	Button(int x, int y)
	{
	    position.x = x;
        position.y = y;
	}

	void SetPosition(int x, int y)
	{
	    position.x = x;
        position.y = y;
	}

	bool IsInside(SDL_Event *e, int Type){
        if (e->type == SDL_MOUSEMOTION || e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_MOUSEBUTTONUP)
        {
            int x, y;
            int button_width, button_height;
            if (Type == 1)
            {
                button_width = BUTTON_TYPE1_WIDTH;
                button_height = BUTTON_TYPE1_HEIGHT;
            }
            else
            {
                button_width = BUTTON_TYPE2_WIDTH;
                button_height = BUTTON_TYPE2_HEIGHT;
            }

            SDL_GetMouseState(&x, &y);

            bool inside = true;
            if (x < position.x)
            {
                inside = false;
            }
            else if (x > position.x + button_width)
            {
                inside = false;
            }
            else if (y < position.y)
            {
                inside = false;
            }
            else if (y > position.y + button_height)
            {
                inside = false;
            }
            return inside;
        }
        return false;
	}

	void Render(SDL_Rect* currentClip, SDL_Renderer* gRenderer, texture gButtonTexture)
	{
        gButtonTexture.render(position.x, position.y, gRenderer, currentClip);
    }
private:
	SDL_Point position;
};

bool checkCollision( SDL_Rect a, SDL_Rect b )
{
    int leftA, leftB;
    int rightA, rightB;
    int topA, topB;
    int bottomA, bottomB;

    leftA = a.x;
    rightA = a.x + a.w;
    topA = a.y;
    bottomA = a.y + a.h;


    leftB = b.x;
    rightB = b.x + b.w;
    topB = b.y;
    bottomB = b.y + b.h;

    if( bottomA <= topB )
    {
        return false;
    }

    if( topA >= bottomB )
    {
        return false;
    }

    if( rightA <= leftB )
    {
        return false;
    }

    if( leftA >= rightB )
    {
        return false;
    }

    return true;
}

string IntToString(int X)
{
    string res = "";
    while(X)
    {
        int x = X % 10;
        char c = x + '0';
        res = c + res;
        X /= 10;
    }
    return res;
}


///CHARACTER
texture AKALI_FRAME[5];
SDL_Rect* AKALI_RECT[5];
SDL_Rect* AKALI_HP_BAR;
SDL_Rect* AKALI_HP;

SDL_Rect* SHURIKEN_RECT[1000500];
SDL_Rect* FIREBALL_RECT[1000500];

SDL_Rect* DRAGON_RECT[6];
texture DRAGON_FRAME[6];
SDL_Rect* DRAGON_HP_BAR;

texture DRAGON_DEATH_FRAME[7];
SDL_Rect* DRAGON_DEATH_Rect[7];

///BUTTON

bool MOUSE_START;
bool MOUSE_HTP;

Button BUTTON_START(600,150);
Button BUTTON_HTP(600,300);
Button BUTTON_QUIT(0, 0);
Button BUTTON_CONTINUE(600,250);
Button BUTTON_BACK(0, 0);
Button QUIT_GAME(600,450);

SDL_Rect* BUTTON_START_R[2];
SDL_Rect* BUTTON_HTP_R[2];
SDL_Rect* BUTTON_QUIT_R[2];
SDL_Rect* BUTTON_BACK_R;
SDL_Rect* BUTTON_CONTINUE_R;
SDL_Rect* QUIT_GAME_R;

texture BUTTON_START_TEXT[2];
texture BUTTON_HTP_TEXT[2];
texture BUTTON_QUIT_TEXT[2];
texture BUTTON_BACK_TEXT;
texture BUTTON_CONTINUE_TEXT;
texture QUIT_GAME_TEXT;
texture HTP;
texture PAUSE_SCR;

texture MENU_BG;
texture win;
texture lose;
texture BG;

texture DRAGON_TEXT;
texture AKALI_TEXT;

void HandleMenuButton(SDL_Event* e, bool& play, bool& help, bool& quit)
{
    if (e->type == SDL_MOUSEMOTION)
    {
        if (BUTTON_START.IsInside(e, 1))
            MOUSE_START = true;
        else MOUSE_START = false;

        if (BUTTON_HTP.IsInside(e, 1))
            MOUSE_HTP = true;
        else MOUSE_HTP = false;
    }

    if (e->type == SDL_MOUSEBUTTONDOWN)
    {
        if (BUTTON_START.IsInside(e, 1))
            play = true;
        if (BUTTON_HTP.IsInside(e, 1))
            help = true;
        if (BUTTON_QUIT.IsInside(e,1))
            quit = true;
    }
}

void HandleBack(SDL_Event *e, bool &help)
{
    if (e->type == SDL_MOUSEBUTTONDOWN)
    {
        if (BUTTON_BACK.IsInside(e, 1))
            help = false;
    }
}

void HandlePause(SDL_Event *e, bool &pause, bool &quit)
{
    if (e -> type == SDL_KEYDOWN && e ->key.keysym.sym == SDLK_ESCAPE)
        pause = true;
    if (pause)
    {
        if (e->type == SDL_MOUSEBUTTONDOWN && BUTTON_CONTINUE.IsInside(e, 1))
            pause = false;

        if (e->type == SDL_MOUSEBUTTONDOWN && BUTTON_QUIT.IsInside(e, 1))
            quit = true;
    }
}

void HandleEnd(SDL_Event* e, bool &quit)
{
    if (e->type == SDL_MOUSEBUTTONDOWN && QUIT_GAME.IsInside(e, 1))
        quit = true;
}

int Time = 0;
int FIREBALL_CD = 0;

bool loadMedia()
{
    texture ttx;

    bool success = true;

    int imgFlags = IMG_INIT_PNG;
    if( !( IMG_Init( imgFlags ) & imgFlags ) )
    {
        printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
        success = false;
    }

    if( TTF_Init() == -1 )
    {
        printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
        success = false;

    }
    gFont = TTF_OpenFont("MY_FONT.ttf", 28);
    if( gFont == NULL )
    {
        printf( "Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError() );
        success = false;
    }
    else
    {
        SDL_Color textColor = { 0, 0, 0 };
        if( !ttx.loadFromRenderedText( "BROWN FOX" , textColor ) )
        {
            printf( "Failed to render text texture!\n" );
            success = false;
        }
    }

    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
                {
                    printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
                    success = false;
                }

    GMusic = Mix_LoadMUS("theme.wav");

    ///load text
    BG.loadFromFile("img/BACKGROUND.png");
    DRAGON_TEXT = renderFromText("DRAGON", "MY_FONT.ttf", 30);
    AKALI_TEXT = renderFromText("PLAYER","MY_FONT.ttf",30);

    AKALI_FRAME[0].loadFromFile("img/AKALI_1.png");
    AKALI_FRAME[1].loadFromFile("img/AKALI_2.png");
    AKALI_FRAME[2].loadFromFile("img/AKALI_3.png");
    AKALI_FRAME[3].loadFromFile("img/AKALI_4.png");
    AKALI_FRAME[4].loadFromFile("img/AKALI_5.png");

    DRAGON_FRAME[0].loadFromFile("img/DRAGON_1.png");
    DRAGON_FRAME[1].loadFromFile("img/DRAGON_2.png");
    DRAGON_FRAME[2].loadFromFile("img/DRAGON_3.png");
    DRAGON_FRAME[3].loadFromFile("img/DRAGON_4.png");
    DRAGON_FRAME[4].loadFromFile("img/DRAGON_5.png");
    DRAGON_FRAME[5].loadFromFile("img/DRAGON_6.png");

    DRAGON_DEATH_FRAME[0].loadFromFile("img/DRAGON_DEATH_1.png");
    DRAGON_DEATH_FRAME[1].loadFromFile("img/DRAGON_DEATH_2.png");
    DRAGON_DEATH_FRAME[2].loadFromFile("img/DRAGON_DEATH_3.png");
    DRAGON_DEATH_FRAME[3].loadFromFile("img/DRAGON_DEATH_4.png");
    DRAGON_DEATH_FRAME[4].loadFromFile("img/DRAGON_DEATH_5.png");
    DRAGON_DEATH_FRAME[5].loadFromFile("img/DRAGON_DEATH_6.png");
    DRAGON_DEATH_FRAME[6].loadFromFile("img/DRAGON_DEATH_7.png");

    BUTTON_START_TEXT[0].loadFromFile("img/START_BUTTON1.png");
    BUTTON_START_TEXT[1].loadFromFile("img/START_BUTTON2.png");

    BUTTON_HTP_TEXT[0].loadFromFile("img/HTP_BUTTON1.png");
    BUTTON_HTP_TEXT[1].loadFromFile("img/HTP_BUTTON2.png");
    HTP.loadFromFile("img/HTP.png");

    BUTTON_BACK_TEXT.loadFromFile("img/BACK_BUTTON.png");
    BUTTON_QUIT_TEXT[0].loadFromFile("img/QUIT_BUTTON.png");

    PAUSE_SCR.loadFromFile("img/PAUSE_SCREEN.png");
    BUTTON_CONTINUE_TEXT.loadFromFile("img/CONTINUE_BUTTON.png");
    BUTTON_QUIT_TEXT[1].loadFromFile("img/QUIT_BUTTON_P.png");
    QUIT_GAME_TEXT.loadFromFile("img/QUIT_BUTTON.png");

    win.loadFromFile("img/VICTORY.png");
    lose.loadFromFile("img/DEFEAT.png");

    return success;
}

int main(int arc, char* argv[])
{
    initSDL(window,renderer);

    bool runable = loadMedia();
    int scrollingOffset = 0;

    //////////////////////PLAYER_SETUP////////////////////
    int max_HP = 200;
    int DAME = 50;
    /////////////////////DRAGON_SETUP////////////////////
    int DR_MAX_HP = 500;
    int DR_DAME = 50;
    /////////////////////////////////////////////////////

    SDL_Event e;
    bool quit = false;
    bool pause = false;
    bool WIN = false;
    bool LOSE = false;

    Character AKALI(0,420, max_HP, DAME);
    AKALI.HPSETUP(max_HP, 50, 200, 50);
    AKALI.change_HP_DF(50);
    AKALI.setCollider(64,91);

    int akali_frame = 0;

    Character DRAGON(1150, 170, DR_MAX_HP, DR_DAME);
    DRAGON.HPSETUP(DR_MAX_HP, 50, 880, 50);
    DRAGON.change_HP_DF(50);
    DRAGON.setCollider(300,200);

    int frame = 0;

    int last_frame = 0;
    Character DRAGON_DEATH(1150, 180);
    DRAGON_DEATH.SetVel(0, 10);

    bool UpDown;
    bool LeftRight;
    bool srk = false;
    vector <Character> QUEUE_RENDER;
    vector <texture> QUEUE_TEXT;

    bool PLAY = false;
    bool HELP = false;
    MENU_BG.loadFromFile("img/BG_MENU.png");
    Mix_PlayMusic(GMusic, -1);
    while (!quit)
    {

        srk = false;
        if (!PLAY && !LOSE && !WIN)
        {
            MENU_BG.render(0, 0, renderer);
            if (!HELP)
            {
                if (MOUSE_START == false)
                    BUTTON_START.Render(BUTTON_START_R[0], renderer, BUTTON_START_TEXT[0]);
                else BUTTON_START.Render(BUTTON_START_R[1], renderer, BUTTON_START_TEXT[1]);

                if (MOUSE_HTP == false)
                    BUTTON_HTP.Render(BUTTON_HTP_R[0], renderer, BUTTON_HTP_TEXT[0]);
                else BUTTON_HTP.Render(BUTTON_HTP_R[1], renderer, BUTTON_HTP_TEXT[1]);

                BUTTON_QUIT.Render(BUTTON_QUIT_R[0], renderer, BUTTON_QUIT_TEXT[0]);
            }
            else
            {
                HTP.render(0, 0, renderer);
                BUTTON_BACK.Render(BUTTON_BACK_R, renderer, BUTTON_BACK_TEXT);
            }
        }
        if (WIN)
        {
            win.render(0, 0, renderer);
            QUIT_GAME.Render(QUIT_GAME_R, renderer, QUIT_GAME_TEXT);
        }
        if (LOSE)
        {
            lose.render(0,0, renderer);
            QUIT_GAME.Render(QUIT_GAME_R, renderer, QUIT_GAME_TEXT);
        }
        while( SDL_PollEvent( &e ) != 0 )
                {
                    if( e.type == SDL_QUIT )
                    {
                        quit = true;
                    }
                    if (!PLAY && !HELP)
                        HandleMenuButton(&e, PLAY, HELP, quit);
                    else HandleBack(&e, HELP);

                    if (PLAY && !pause)
                        AKALI.HandleEvent(e, UpDown, LeftRight, srk);

                    if (PLAY)
                        HandlePause(&e, pause,quit);

                    if (!AKALI.live())
                    {
                        PLAY = false;
                        LOSE = true;
                        HandleEnd(&e, quit);
                    }

                    if (!DRAGON.live())
                    {
                        if (last_frame == 90)
                            PLAY = false;
                        WIN = true;
                        HandleEnd(&e, quit);
                    }

                    if (pause)
                    {
                        BUTTON_QUIT.SetPosition(600,400);
                        PAUSE_SCR.render(300, 100, renderer);
                        BUTTON_CONTINUE.Render(BUTTON_CONTINUE_R, renderer, BUTTON_CONTINUE_TEXT);
                        BUTTON_QUIT.Render(BUTTON_QUIT_R[1], renderer, BUTTON_QUIT_TEXT[1]);
                        HandlePause(&e, pause, quit);
                    }
                }
        if (PLAY)
        {
            if (!pause)
            {
                AKALI.Move();
                AKALI.UP();
                AKALI.CDR();
                SDL_RenderClear( renderer );
                scrollingOffset --;
                if (scrollingOffset < -4200)
                    scrollingOffset = 0;
                BG.render( scrollingOffset, 0, renderer);
                BG.render( scrollingOffset + 4200, 0, renderer);

                if (AKALI.live())
                    AKALI.Render(AKALI_RECT[akali_frame / 10], renderer, AKALI_FRAME[akali_frame / 10]);

                if (DRAGON.live())
                    DRAGON.Render(DRAGON_RECT[frame / 10], renderer, DRAGON_FRAME[frame / 10]);

                if (!DRAGON.live())
                {
                    DRAGON_DEATH.Render(DRAGON_DEATH_Rect[last_frame / 15], renderer, DRAGON_DEATH_FRAME[last_frame / 15]);
                    if (last_frame / 15 < 6)
                        last_frame++;
                    DRAGON_DEATH.FALL(300);
                }
                ++akali_frame;
                ++frame;
                Time++;
                FIREBALL_CD++;

                if (FIREBALL_CD >= 50)
                    FIREBALL_CD = 0;

                if (Time >= 24)
                    Time -= 24;

                if( frame / 10 >= 6)
                    frame = 0;

                if (akali_frame / 10 > 4)
                    akali_frame = 0;

                if (FIREBALL_CD == 0 && DRAGON.live() && AKALI.live())
                {
                    texture fireball("img/fire_ball.png");
                    Character FIREBALL(DRAGON.GetPosX() - 50, DRAGON.GetPosY() + 70, 0, DRAGON.DAMAGE());
                    FIREBALL.type = "FB";
                    FIREBALL.setCollider(50, 50);
                    int RAN = rand() % 7;
                    switch(RAN)
                    {
                        case 0:
                            FIREBALL.SetVel(-10,10);
                            break;
                        case 1:
                            FIREBALL.SetVel(-10,3);
                            break;
                        case 2:
                            FIREBALL.SetVel(-15,10);
                            break;
                        case 3:
                            FIREBALL.SetVel(-10,5);
                            break;
                        case 4:
                            FIREBALL.SetVel(-15,5);
                            break;
                        case 5:
                            FIREBALL.SetVel(-15,2);
                            break;
                        case 6:
                            FIREBALL.SetVel(-10,2);
                    }

                    QUEUE_TEXT.push_back(fireball);
                    QUEUE_RENDER.push_back(FIREBALL);
                }

                if (srk == true)
                {
                    texture shuriken("img/attack2.png");
                    Character SHURIKEN(AKALI.GetPosX() + 60, AKALI.GetPosY() + 45, 0, AKALI.DAMAGE());
                    SHURIKEN.type = "SRK";
                    SHURIKEN.setCollider(30, 30);
                    SHURIKEN.SetVel(10,0);
                    QUEUE_TEXT.push_back(shuriken);
                    QUEUE_RENDER.push_back(SHURIKEN);
                }

                for (int i = 0;i < (int) QUEUE_RENDER.size();i++)
                {
                    Character obj = QUEUE_RENDER[i];
                    if (QUEUE_RENDER[i].type == "SRK")
                    {
                        if (obj.GetPosX() < SCREEN_WIDTH - 60 && obj.GetHit() == false)
                            obj.Render(SHURIKEN_RECT[i], renderer, QUEUE_TEXT[i], Time * 15.0);

                        if (AKALI.live() && obj.GetHit() == false && checkCollision(DRAGON.GetRect(), obj.GetRect()) == true)
                        {
                            DRAGON.REDUCE_HP(obj.DAMAGE());
                            obj.HIT();
                        }
                    }
                    else
                    {
                        if (obj.GetPosX() > 60 && obj.GetPosY() < 600 && obj.GetHit() == false)
                            obj.Render(FIREBALL_RECT[i], renderer, QUEUE_TEXT[i]);

                        if (DRAGON.live() && obj.GetHit() == false && checkCollision(AKALI.GetRect(), obj.GetRect()) == true)
                        {
                            AKALI.REDUCE_HP(obj.DAMAGE());
                            obj.HIT();
                        }
                        obj.FALL(600);
                    }
                    obj.Move();
                    QUEUE_RENDER[i] = obj;
                }

                if (AKALI.live())
                {
                    AKALI_TEXT.render(25,50, renderer);
                    AKALI.Render_HP(AKALI_HP_BAR, renderer);
                    string AKALI_HP = IntToString(AKALI.GET_HEALTH()) + "/" + IntToString(max_HP);
                    texture AKALI_NOW_HP;
                    AKALI_NOW_HP = renderFromText(AKALI_HP, "MY_FONT.ttf", 20);
                    AKALI_NOW_HP.render(200,60, renderer);
                }

                if (DRAGON.live())
                {
                    DRAGON_TEXT.render(680, 50, renderer);
                    DRAGON.Render_HP(DRAGON_HP_BAR, renderer);
                    string DRAGON_HP = IntToString(DRAGON.GET_HEALTH()) + "/" + IntToString(DR_MAX_HP);
                    texture DRAGON_NOW_HP;
                    DRAGON_NOW_HP = renderFromText(DRAGON_HP, "MY_FONT.ttf", 20);
                    DRAGON_NOW_HP.render(880,60, renderer);
                }
            }

        }
        SDL_RenderPresent( renderer );
    }

    quitSDL(window, renderer);
    return 0;
}
