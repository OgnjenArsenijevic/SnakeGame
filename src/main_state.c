#include <main_state.h>
#include <glad/glad.h>
#include <math.h>
#include <rafgl.h>
#include <game_constants.h>
#include <time.h>
#include <math.h>
#include <string.h>
#define MAX_SNAKE_SIZE 10000
#define STATE_MAIN_MENU 1
#define STATE_GAME 2
#define STATE_PAUSE 3
#define STATE_ENDGAME 4
#define MAX_PARTICLES 500
#define LEVEL_SIZE 9
#define SNAKE_OFFSET 50
#define COLOR1 rafgl_RGB(255,204,229)
#define COLOR2 rafgl_RGB(153,255,255)
#define COLOR3 rafgl_RGB(178,102,255)
#define COLOR4 rafgl_RGB(255,255,51)
#define COLOR5 rafgl_RGB(204,0,102)
#define COLOR6 rafgl_RGB(102,102,0)
#define COLOR7 rafgl_RGB(153,0,0)
#define COLOR8 rafgl_RGB(51,0,51)

static rafgl_raster_t doge;
static rafgl_raster_t upscaled_doge;
static rafgl_raster_t raster, raster2;
static rafgl_raster_t checker;

static rafgl_raster_t mainMenu;
static rafgl_raster_t pause;
static rafgl_raster_t newHighScore;
static rafgl_raster_t gameImage;
static rafgl_raster_t coinImage;
static rafgl_raster_t gameOverImage;
static rafgl_raster_t highScoreImage;
static rafgl_raster_t mainPageImage;
static rafgl_raster_t pauseImage;
static rafgl_texture_t texture;



static int raster_width = RASTER_WIDTH, raster_height = RASTER_HEIGHT;

static char save_file[256];
int save_file_no = 0;

typedef struct _particle_t
{
    float x, y, dx, dy;
    int life;


} particle_t;


///my code
int lerp(int l, int r, float s)
{
    return l + (r - l) * s;
}

rafgl_pixel_rgb_t lerp_pixel(rafgl_pixel_rgb_t l, rafgl_pixel_rgb_t r, float s)
{
    rafgl_pixel_rgb_t result;
    result.r = lerp(l.r, r.r, s);
    result.g = lerp(l.g, r.g, s);
    result.b = lerp(l.b, r.b, s);
    result.a = lerp(l.a, r.a, s);
    return result;
}

int currState=STATE_MAIN_MENU;
int inputLockFlag=0;

typedef struct button
{
    rafgl_raster_t image;
    int startX, startY, w, h, state;

}Button;

typedef struct square
{
    int x,y;
}Square;

typedef struct snake
{
    int posX, posY, speed, scale, bodySize, score, dirX, dirY;
    Square body[MAX_SNAKE_SIZE];
}Snake;

typedef struct fruit
{
    int posX, posY, scale;
}Fruit;

Snake player;
Fruit food;
rafgl_spritesheet_t sprite;
rafgl_spritesheet_t spriteCoin;
rafgl_spritesheet_t spriteGameOver;
rafgl_spritesheet_t spriteHighScore;
rafgl_spritesheet_t spriteMainPage;
rafgl_spritesheet_t spritePause;
int newHighScoreSet=0;
int score=0;
int minusColor=20;
int changeColor=1;
int coinFrame=0;
int level=1;
int highScore=0;
int eatScore=1;
int currLevelProgress=0;
int randomArray[8]={0,1,2,3,4,5,6,7};
float deltaTimeCheck=0.1f;
Button endlessBtn;
Button normalBtn;
Button quitBtn;
Button retryBtn;
Button mainMenuBtn;
rafgl_pixel_rgb_t left_colour, right_colour;

particle_t particles[MAX_PARTICLES];

void draw_particles(rafgl_raster_t *raster)
{
    int i;
    particle_t p;
    for(i = 0; i < MAX_PARTICLES; i++)
    {
        p = particles[i];
        if(p.life <= 0) continue;
        rafgl_raster_draw_line(raster, p.x - p.dx, p.y - p.dy, p.x, p.y, rafgl_RGB(rand()%256, rand()%256,  rand()%256));
    }
}

static float elasticity = 0.6;

void update_particles(float delta_time)
{
    int i;
    for(i = 0; i < MAX_PARTICLES; i++)
    {
        if(particles[i].life <= 0) continue;

        particles[i].life-=delta_time;

        particles[i].x += particles[i].dx * delta_time * 20;
        particles[i].y += particles[i].dy * delta_time * 20;
        particles[i].dx *= 0.995f;
        particles[i].dy *= 0.995f;
        particles[i].dy += 0.05;

        if(particles[i].x < 0)
        {
            particles[i].x = 0;
            particles[i].dx = (rafgl_abs_m(particles[i].dx)) * randf() * elasticity;
        }

        if(particles[i].y < 0)
        {
            particles[i].y = 0;
            particles[i].dy = (rafgl_abs_m(particles[i].dy)) * randf() * elasticity;
        }

        if(particles[i].x >= raster_width)
        {
            particles[i].x = raster_width - 1;
            particles[i].dx = (rafgl_abs_m(particles[i].dx)) * randf() * (-elasticity);
        }

        if(particles[i].y >= raster_height)
        {
            particles[i].y = raster_height - 1;
            particles[i].dy = (rafgl_abs_m(particles[i].dy)) * randf() * (-elasticity);
        }

    }
}

void createFirework(float deltaTime)
{
    int i, gen = 144, radius = 10;
    float angle, speed;
    int n,m;
    m = rand()%raster_width;
    n = rand()%raster_height;
    for(i = 0; (i < MAX_PARTICLES) && gen; i++)
    {
        if(particles[i].life <= 0)
        {
            particles[i].life = 100 * randf() + 100;
            particles[i].x = m;
            particles[i].y = n;

            angle = randf() * M_PI *  2.0f;
            speed = ( 0.3f + 0.7 * randf()) * radius;
            particles[i].dx = cosf(angle) * speed;
            particles[i].dy = sinf(angle) * speed;
            gen--;

        }
    }

}

void spawnFruit()
{
    int korX=(rand()%(raster_width/food.scale))*food.scale;
    int korY=(rand()%(raster_height/food.scale))*food.scale;
    if(korX==player.posX && korY==player.posY)
    {
        spawnFruit();
        return;
    }
    for(int i=0;i<player.bodySize;i++)
        if(korX==player.body[i].x && korY==player.body[i].y)
        {
            spawnFruit();
            return;
        }
    food.posX=korX;
    food.posY=korY;
    return;
}

void handleInput(rafgl_game_data_t *gameData)
{
    if(gameData->is_lmb_down)
        onClick(gameData);
    if(gameData->keys_pressed[RAFGL_KEY_DOWN] || gameData->keys_pressed[RAFGL_KEY_S])
    {
        if(player.dirY==0 && !inputLockFlag)
        {
            player.dirX=0;
            player.dirY=1;
            inputLockFlag=1;
        }
    }
    if(gameData->keys_pressed[RAFGL_KEY_UP] || gameData->keys_pressed[RAFGL_KEY_W])
    {
        if(player.dirY==0 && !inputLockFlag)
        {
            player.dirX=0;
            player.dirY=-1;
            inputLockFlag=1;
        }
    }
    if(gameData->keys_pressed[RAFGL_KEY_LEFT] || gameData->keys_pressed[RAFGL_KEY_A])
    {
        if(player.dirX==0 && !inputLockFlag)
        {
            player.dirX=-1;
            player.dirY=0;
            inputLockFlag=1;
        }
    }
    if(gameData->keys_pressed[RAFGL_KEY_RIGHT] || gameData->keys_pressed[RAFGL_KEY_D])
    {
        if(player.dirX==0 && !inputLockFlag)
        {
            player.dirX=1;
            player.dirY=0;
            inputLockFlag=1;
        }
    }
    if(gameData->keys_pressed[RAFGL_KEY_P] && currState==STATE_GAME)
        currState=STATE_PAUSE;
    if(gameData->keys_pressed[RAFGL_KEY_U] && currState==STATE_PAUSE)
        currState=STATE_GAME;
    return;
}

void initButtons()
{
    rafgl_raster_t endless;
    rafgl_raster_load_from_image(&endless, "res/images/endlessBtn.png");
    endlessBtn.image=endless;
    endlessBtn.h=endless.height;
    endlessBtn.w=endless.width;
    //printf("%d %d\n",endless.height,endless.width);
    endlessBtn.startX=raster_width/2-endless.width/2;
    endlessBtn.startY=raster_height/3;
    endlessBtn.state=STATE_MAIN_MENU;
   /* rafgl_raster_t normal;
    rafgl_raster_load_from_image(&normal, "res/images/normalBtn.png");
    normalBtn.image=normal;
    normalBtn.h=normal.height;
    normalBtn.w=normal.width;
   // printf("%d %d\n",endless.height,endless.width);
    normalBtn.startX=raster_width/2-normal.width/2;
    normalBtn.startY=raster_height/4+endless.height+30;
    normalBtn.state=STATE_MAIN_MENU;*/
    rafgl_raster_t quit;
    rafgl_raster_load_from_image(&quit, "res/images/quitBtn.png");
    quitBtn.image=quit;
    quitBtn.h=quit.height;
    quitBtn.w=quit.width;
   // printf("%d %d\n",endless.height,endless.width);
    quitBtn.startX=raster_width/2-quit.width/2;
    quitBtn.startY=raster_height/3+endless.height+30;
    quitBtn.state=STATE_MAIN_MENU;

    rafgl_raster_t retry;
    rafgl_raster_load_from_image(&retry, "res/images/retryBtn.png");
    retryBtn.image=retry;
    retryBtn.h=retry.height;
    retryBtn.w=retry.width;
   // printf("%d %d\n",endless.height,endless.width);
    retryBtn.startX=raster_width/2-retry.width/2;
    retryBtn.startY=raster_height/3;
    retryBtn.state=STATE_ENDGAME;
    rafgl_raster_t mainMenu;
    rafgl_raster_load_from_image(&mainMenu, "res/images/mainMenuBtn.png");
    mainMenuBtn.image=mainMenu;
    mainMenuBtn.h=mainMenu.height;
    mainMenuBtn.w=mainMenu.width;
   // printf("%d %d\n",endless.height,endless.width);
    mainMenuBtn.startX=raster_width/2-mainMenu.width/2;
    mainMenuBtn.startY=raster_height/3+retry.height+30;
    mainMenuBtn.state=STATE_ENDGAME;
}

void drawButton(Button btn)
{
    if(btn.state!=currState)
        return;
    for(int j=btn.startY;j<btn.startY+btn.h;j++)
        for(int i=btn.startX;i<btn.startX+btn.w;i++)
        {
            //printf("%d %d\n",btn.image.height,btn.image.width);
            //printf("%d %d\n",i-btn.startX,j-btn.startY);
            pixel_at_m(raster,i,j)=pixel_at_m(btn.image,i-btn.startX,j-btn.startY);
        }
}

void drawHoverButton(Button btn)
{
    if(btn.state!=currState)
        return;
    for(int j=btn.startY;j<btn.startY+btn.h;j++)
        for(int i=btn.startX;i<btn.startX+btn.w;i++)
        {
            //printf("%d %d\n",btn.image.height,btn.image.width);
            //printf("%d %d\n",i-btn.startX,j-btn.startY);
            pixel_at_m(raster,i,j)=pixel_at_m(btn.image,i-btn.startX,j-btn.startY);
            pixel_at_m(raster,i,j).r=255-pixel_at_m(raster,i,j).r;
            pixel_at_m(raster,i,j).g=255-pixel_at_m(raster,i,j).g;
            pixel_at_m(raster,i,j).b=255-pixel_at_m(raster,i,j).b;
            pixel_at_m(raster,i,j).a=255-pixel_at_m(raster,i,j).a;
        }
}

float deltaClickTime = 1;

void onClick(rafgl_game_data_t* gameData)
{
    if (deltaClickTime > 0)
        return;
    if(checkHover(endlessBtn,gameData))
    {
        currState=STATE_GAME;
        deltaClickTime = 1;
    }
    else if(checkHover(quitBtn,gameData))
    {
        exit(0);
        deltaClickTime = 1;
    }
    else if(checkHover(retryBtn,gameData))
    {
        currState=STATE_GAME;
        deltaClickTime = 1;
    }
    else if(checkHover(mainMenuBtn,gameData))
    {
        currState=STATE_MAIN_MENU;
        deltaClickTime = 1;
    }
}

int checkHover(Button btn, rafgl_game_data_t* gameData)
{
    if(btn.state!=currState)
        return 0;
    double mouseX=gameData->mouse_pos_x;
    double mouseY=gameData->mouse_pos_y;
    //printf("%f %d\n", mouseX,btn.startX);
    return (mouseX>=btn.startX && mouseX<btn.startX+btn.w && mouseY>=btn.startY && mouseY<btn.startY+btn.h);
}

void initSnake()
{
    player.bodySize=1;
    player.score=0;
    player.dirX=0;
    player.dirY=0;
    player.speed=32;
    player.scale=32;
    player.posX=(raster_width/player.scale)/2*player.scale;
    player.posY=(raster_height/player.scale)/2*player.scale;
    for(int i=0;i<MAX_SNAKE_SIZE;i++)
    {
        player.body[i].x=-1e9;
        player.body[i].y=-1e9;
    }
    player.body[0].x=player.posX;
    player.body[0].y=player.posY+player.scale;
    return;
}

void updateSnake()
{
    if(player.dirX==0 && player.dirY==0) return;
    if(player.bodySize>0)
    {
        for(int i=player.bodySize-1;i>0;i--)
        {
            player.body[i].x=player.body[i-1].x;
            player.body[i].y=player.body[i-1].y;
        }
        player.body[0].x=player.posX;
        player.body[0].y=player.posY;
    }

    player.posX+=player.dirX*player.scale;
    player.posY+=player.dirY*player.scale;
    player.posX=(player.posX+raster_width)%raster_width;
    player.posY=(player.posY+raster_height)%raster_height;
    // printf("%d %d\n",player.posX,raster_width);
    return;
}

void eat()
{
    if(player.posX==food.posX && player.posY==food.posY)
    {
        //printf("debug\n");
        if(score%5==0 && score>0)
            player.score+=eatScore;
        player.score+=eatScore;
        currLevelProgress++;
        currLevelProgress%=LEVEL_SIZE;
        player.bodySize++;
        score++;
        if(score%LEVEL_SIZE==0 && score>0)
        {
            level++;
            eatScore++;
            for(int i=0;i<8;i++) ///random shuffle array
            {
                int j,tmp;
                j=rand()%(8-i)+i;
                tmp=randomArray[j];
                randomArray[j]=randomArray[i];
                randomArray[i]=tmp; // Swap i and j
            }
            if(deltaTimeCheck>0.03f)
                deltaTimeCheck-=0.0065f;
        }
        spawnFruit();
        if(score%5==0)
            minusColor+=20;
    }
}

void checkDeath()
{
    for(int i=0;i<player.bodySize;i++)
        if(player.posX==player.body[i].x && player.posY==player.body[i].y)
        {
            //printf("%d\n",i);
            //printf("%d %d\n",player.body[i].x, player.body[i].y);
            if(player.score>highScore)
            {
                highScore=player.score;
                newHighScoreSet=1;
                char buff[10];
                FILE *file;
                itoa(highScore,buff,10);
                file = fopen("res/files/HighScore.txt", "w");
                fputs(buff, file);
                fclose(file);
            }
            else
                newHighScoreSet=0;
            initSnake();
            spawnFruit();
            score=0;
            minusColor=0;
            eatScore=1;
            level=1;
            currLevelProgress=0;
            deltaTimeCheck=0.1f;
            for(int i=0;i<8;i++) ///random shuffle array
            {
                int j,tmp;
                j=rand()%(8-i)+i;
                tmp=randomArray[j];
                randomArray[j]=randomArray[i];
                randomArray[i]=tmp; // Swap i and j
            }
            currState=STATE_ENDGAME;
            return;
        }
}

void drawSnake()
{
    rafgl_pixel_rgb_t pixel;
    pixel.rgba=rafgl_RGB(255,255,255);
    int prevX=player.posX, prevY=player.posY;
    if(player.dirX==-1) //head-left
        drawFromSpritesheet(player.posX,player.posY,player.scale,player.scale,sprite,1,3);
    else if(player.dirX==1) //head-right
        drawFromSpritesheet(player.posX,player.posY,player.scale,player.scale,sprite,0,4);
    else if(player.dirY==-1) //head-top
        drawFromSpritesheet(player.posX,player.posY,player.scale,player.scale,sprite,0,3);
    else if(player.dirY==1) //head-down
        drawFromSpritesheet(player.posX,player.posY,player.scale,player.scale,sprite,1,4);
    else
        drawFromSpritesheet(player.posX,player.posY,player.scale,player.scale,sprite,0,3);
        //drawRectangle(player.posX,player.posY,player.scale,player.scale,pixel);
    for(int i=0;i<player.bodySize-1;i++)
    {
        //printf("%d %d\n",prevX,prevY);
        if(player.body[i].x==0 && player.body[i].y==0 && !((prevX==0+player.scale && prevY==0) || (prevX==0 && prevY==0+player.scale)) && !((player.body[i+1].x==0+player.scale && player.body[i+1].y==0) || (player.body[i+1].x==0 && player.body[i+1].y==0+player.scale))) //top-left-corner
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,2,2);
        else if(player.body[i].x==raster_width-player.scale && player.body[i].y==0 && !((prevX==raster_width-2*player.scale && prevY==0) || (prevX==raster_width-player.scale && prevY==0+player.scale)) && !((player.body[i+1].x==raster_width-2*player.scale && player.body[i+1].y==0) || (player.body[i+1].x==raster_width-player.scale && player.body[i+1].y==0+player.scale))) //top-right-corner
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,1,0);
        else if(player.body[i].x==0 && player.body[i].y==raster_height-player.scale && !((prevX==0+player.scale && prevY==raster_height-player.scale) || (prevX==0 && prevY==raster_height-2*player.scale)) && !((player.body[i+1].x==0+player.scale && player.body[i+1].y==raster_height-player.scale) || (player.body[i+1].x==0 && player.body[i+1].y==raster_height-2*player.scale))) //bottom-left-corner
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,0,2);
        else if(player.body[i].x==raster_width-player.scale && player.body[i].y==raster_height-player.scale && !((prevX==raster_width-2*player.scale && prevY==raster_height-player.scale) || (prevX==raster_width-player.scale && prevY==raster_height-2*player.scale)) && !((player.body[i+1].x==raster_width-2*player.scale && player.body[i+1].y==raster_height-player.scale) || (player.body[i+1].x==raster_width-player.scale && player.body[i+1].y==raster_height-2*player.scale))) //bottom-right-corner
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,0,0);
        else if(player.body[i].x==player.body[i+1].x && player.body[i].y==0 && player.body[i+1].y==raster_height-player.scale && player.body[i].x<prevX && player.body[i].y==prevY) //enter-top-edge-right
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,1,0);
        else if(player.body[i].x==player.body[i+1].x && player.body[i].y==0 && player.body[i+1].y==raster_height-player.scale && player.body[i].x>prevX && player.body[i].y==prevY) //enter-top-edge-left
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,2,2);
        else if(player.body[i].x==player.body[i+1].x && player.body[i].y==raster_height-player.scale && player.body[i+1].y==0 && player.body[i].x<prevX && player.body[i].y==prevY) //enter-bottom-edge-right
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,0,0);
        else if(player.body[i].x==player.body[i+1].x && player.body[i].y==raster_height-player.scale && player.body[i+1].y==0 && player.body[i].x>prevX && player.body[i].y==prevY) //enter-bottom-edge-left
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,0,2);
        else if(player.body[i].y==player.body[i+1].y && player.body[i].x==0 && player.body[i+1].x==raster_width-player.scale && player.body[i].y>prevY && player.body[i].x==prevX) //enter-left-edge-up
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,2,2);
        else if(player.body[i].y==player.body[i+1].y && player.body[i].x==0 && player.body[i+1].x==raster_width-player.scale && player.body[i].y<prevY && player.body[i].x==prevX) //enter-left-edge-down
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,0,2);
        else if(player.body[i].y==player.body[i+1].y && player.body[i].x==raster_width-player.scale && player.body[i+1].x==0 && player.body[i].y>prevY && player.body[i].x==prevX) //enter-right-edge-up
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,1,0);
        else if(player.body[i].y==player.body[i+1].y && player.body[i].x==raster_width-player.scale && player.body[i+1].x==0 && player.body[i].y<prevY && player.body[i].x==prevX) //enter-right-edge-down
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,0,0);
        else if(player.body[i].x==prevX && player.body[i].y==0 && prevY==raster_height-player.scale && player.body[i].x<player.body[i+1].x && player.body[i].y==player.body[i+1].y) //exit-top-edge-right
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,1,0);
        else if(player.body[i].x==prevX && player.body[i].y==0 && prevY==raster_height-player.scale && player.body[i].x>player.body[i+1].x && player.body[i].y==player.body[i+1].y) //exit-top-edge-left
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,2,2);
        else if(player.body[i].x==prevX && player.body[i].y==raster_height-player.scale && prevY==0 && player.body[i].x<player.body[i+1].x && player.body[i].y==player.body[i+1].y) //exit-bottom-edge-right
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,0,0);
        else if(player.body[i].x==prevX && player.body[i].y==raster_height-player.scale && prevY==0 && player.body[i].x>player.body[i+1].x && player.body[i].y==player.body[i+1].y) //exit-bottom-edge-left
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,0,2);
        else if(player.body[i].y==prevY && player.body[i].x==0 && prevX==raster_width-player.scale && player.body[i].y<player.body[i+1].y && player.body[i].x==player.body[i+1].x) //exit-left-edge-up
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,0,2);
        else if(player.body[i].y==prevY && player.body[i].x==0 && prevX==raster_width-player.scale && player.body[i].y>player.body[i+1].y && player.body[i].x==player.body[i+1].x) //exit-left-edge-down
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,2,2);
        else if(player.body[i].y==prevY && player.body[i].x==raster_width-player.scale && prevX==0 && player.body[i].y<player.body[i+1].y && player.body[i].x==player.body[i+1].x) //exit-right-edge-up
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,0,0);
        else if(player.body[i].y==prevY && player.body[i].x==raster_width-player.scale && prevX==0 && player.body[i].y>player.body[i+1].y && player.body[i].x==player.body[i+1].x) //exit-right-edge-down
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,1,0);
        else if(player.body[i].x<prevX && player.body[i].y==prevY && player.body[i].x==player.body[i+1].x && player.body[i].y<player.body[i+1].y) //up-right
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,0,0);
        else if(player.body[i].x>prevX && player.body[i].y==prevY && player.body[i].x==player.body[i+1].x && player.body[i].y<player.body[i+1].y) //up-left
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,0,2);
        else if(player.body[i].x<prevX && player.body[i].y==prevY && player.body[i].x==player.body[i+1].x && player.body[i].y>player.body[i+1].y) //down-right
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,1,0);
        else if(player.body[i].x>prevX && player.body[i].y==prevY && player.body[i].x==player.body[i+1].x && player.body[i].y>player.body[i+1].y) //down-left
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,2,2);
        else if(player.body[i].x==prevX && player.body[i].y<prevY && player.body[i].x<player.body[i+1].x && player.body[i].y==player.body[i+1].y) //left-down
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,0,0);
        else if(player.body[i].x==prevX && player.body[i].y>prevY && player.body[i].x<player.body[i+1].x && player.body[i].y==player.body[i+1].y) //left-up
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,1,0);
        else if(player.body[i].x==prevX && player.body[i].y<prevY && player.body[i].x>player.body[i+1].x && player.body[i].y==player.body[i+1].y) //right-down
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,0,2);
        else if(player.body[i].x==prevX && player.body[i].y>prevY && player.body[i].x>player.body[i+1].x && player.body[i].y==player.body[i+1].y) //right-up
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,2,2);
        else if(player.body[i].x==prevX && player.body[i].y!=prevY) //up-down
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,1,2);
        else if(player.body[i].x!=prevX && player.body[i].y==prevY) //left-right
            drawFromSpritesheet(player.body[i].x,player.body[i].y,player.scale,player.scale,sprite,0,1);
       // else drawRectangle(player.body[i].x,player.body[i].y,player.scale,player.scale,pixel);
        prevX=player.body[i].x;
        prevY=player.body[i].y;
    }
    if(player.body[player.bodySize-1].x==prevX && player.body[player.bodySize-1].y==0 && prevY==raster_height-player.scale) //tail-up-edge
        drawFromSpritesheet(player.body[player.bodySize-1].x,player.body[player.bodySize-1].y,player.scale,player.scale,sprite,2,3);
    else if(player.body[player.bodySize-1].x==prevX && player.body[player.bodySize-1].y==raster_height-player.scale && prevY==0) //rail-down-edge
        drawFromSpritesheet(player.body[player.bodySize-1].x,player.body[player.bodySize-1].y,player.scale,player.scale,sprite,3,4);
    else if(player.body[player.bodySize-1].y==prevY && player.body[player.bodySize-1].x==0 && prevX==raster_width-player.scale) //tail-left-edge
        drawFromSpritesheet(player.body[player.bodySize-1].x,player.body[player.bodySize-1].y,player.scale,player.scale,sprite,3,3);
    else if(player.body[player.bodySize-1].y==prevY && player.body[player.bodySize-1].x==raster_width-player.scale && prevX==0) //tail-right-edge
        drawFromSpritesheet(player.body[player.bodySize-1].x,player.body[player.bodySize-1].y,player.scale,player.scale,sprite,2,4);
    else if(player.body[player.bodySize-1].x==prevX && player.body[player.bodySize-1].y<prevY) //tail-down
        drawFromSpritesheet(player.body[player.bodySize-1].x,player.body[player.bodySize-1].y,player.scale,player.scale,sprite,3,4);
    else if(player.body[player.bodySize-1].x==prevX && player.body[player.bodySize-1].y>prevY) //tail-up
        drawFromSpritesheet(player.body[player.bodySize-1].x,player.body[player.bodySize-1].y,player.scale,player.scale,sprite,2,3);
    else if(player.body[player.bodySize-1].x<prevX && player.body[player.bodySize-1].y==prevY) //tail-right
        drawFromSpritesheet(player.body[player.bodySize-1].x,player.body[player.bodySize-1].y,player.scale,player.scale,sprite,2,4);
    else if(player.body[player.bodySize-1].x>prevX && player.body[player.bodySize-1].y==prevY) //tail-left
        drawFromSpritesheet(player.body[player.bodySize-1].x,player.body[player.bodySize-1].y,player.scale,player.scale,sprite,3,3);
    else if(player.body[player.bodySize-1].x==prevX && player.body[player.bodySize-1].y==prevY) //up-start
        drawFromSpritesheet(player.body[player.bodySize-1].x,player.body[player.bodySize-1].y,player.scale,player.scale,sprite,2,3);
    return;
}

void drawFood()
{
    rafgl_pixel_rgb_t pixel;
    pixel.rgba=rafgl_RGB(0,0,255);
    //drawRectangle(food.posX,food.posY,food.scale,food.scale,pixel);
    drawFromSpritesheet(food.posX,food.posY,food.scale,food.scale,sprite,3,0);
    return;
}

void drawBackground()
{
    for(int y=0;y<raster_height;y++)
        for(int x=0;x<raster_width;x++)
            pixel_at_m(raster,x,y).rgba=rafgl_RGB(0,0,0);
}

void drawRectangle(int x, int y, int w, int h, rafgl_pixel_rgb_t pixel)
{
    int rX=x+w, rY=y+h;
    x=rafgl_max_m(0,x);
    y=rafgl_max_m(0,y);
    rX=rafgl_min_m(rX,raster_width-1);
    rY=rafgl_min_m(rY,raster_height-1);
    for(int j=y;j<rY;j++)
        for(int i=x;i<rX;i++)
            pixel_at_m(raster,i,j).rgba=pixel.rgba;
    return;
}

void drawFromSpritesheet(int x, int y, int w, int h, rafgl_spritesheet_t sprite, int row, int col)
{
    int rX=x+w, rY=y+h;
    x=rafgl_max_m(0,x);
    y=rafgl_max_m(0,y);
    rX=rafgl_min_m(rX,raster_width-1);
    rY=rafgl_min_m(rY,raster_height-1);
    for(int j=y;j<rY;j++)
        for(int i=x;i<rX;i++)
        {
            if (pixel_at_m(sprite.sheet,col*sprite.sheet_width+(i-x),row*sprite.sheet_height+(j-y)).b<130 && changeColor)
            {
               // pixel_at_m(raster,i,j)=pixel_at_m(sprite.sheet,col*sprite.sheet_width+(i-x),row*sprite.sheet_height+(j-y));
                rafgl_pixel_rgb_t pixel=pixel_at_m(sprite.sheet,col*sprite.sheet_width+(i-x),row*sprite.sheet_height+(j-y));
                if(score%5==0 && changeColor)
                {
                    pixel.r=(pixel.r+256-minusColor)%256;
                    pixel.g=(pixel.g+256-minusColor)%256;
                    pixel.b=(pixel.b+256-minusColor)%256;
                    pixel.a=(pixel.a+256-minusColor)%256;
                }
                pixel_at_m(raster,i,j)=pixel;
            }
            else if(!changeColor)
            {
                rafgl_pixel_rgb_t pixel=pixel_at_m(sprite.sheet,col*sprite.sheet_width+(i-x),row*sprite.sheet_height+(j-y));
                pixel_at_m(raster,i,j)=pixel;
            }
        }
    changeColor=1;
}

void drawFromWholeSprite(int x, int y, rafgl_spritesheet_t sprite, int invert)
{
    int rX=x+sprite.sheet_width, rY=y+sprite.sheet_height;
    x=rafgl_max_m(0,x);
    y=rafgl_max_m(0,y);
    rX=rafgl_min_m(rX,raster_width-1);
    rY=rafgl_min_m(rY,raster_height-1);
    for(int j=y;j<rY;j++)
        for(int i=x;i<rX;i++)
            pixel_at_m(raster,i,j)=pixel_at_m(sprite.sheet,i-x,j-y);
    if(invert)
        for(int j=y;j<rY;j++)
            for(int i=x;i<rX;i++)
            {
                pixel_at_m(raster,i,j).r=(255-pixel_at_m(raster,i,j).r);
                pixel_at_m(raster,i,j).g=(255-pixel_at_m(raster,i,j).g);
                pixel_at_m(raster,i,j).b=(255-pixel_at_m(raster,i,j).b);
            }
    return;
}

void drawPause(int x, int y, rafgl_spritesheet_t sprite)
{
    int rX=x+sprite.sheet_width, rY=y+sprite.sheet_height;
    x=rafgl_max_m(0,x);
    y=rafgl_max_m(0,y);
    rX=rafgl_min_m(rX,raster_width-1);
    rY=rafgl_min_m(rY,raster_height-1);
    for(int j=y;j<rY;j++)
        for(int i=x;i<rX;i++)
            if(pixel_at_m(sprite.sheet,i-x,j-y).r<150 && pixel_at_m(sprite.sheet,i-x,j-y).g<150 && pixel_at_m(sprite.sheet,i-x,j-y).b<150)
            {
                pixel_at_m(raster,i,j)=pixel_at_m(sprite.sheet,i-x,j-y);
                pixel_at_m(raster,i,j).r=(255-pixel_at_m(raster,i,j).r);
                pixel_at_m(raster,i,j).g=(255-pixel_at_m(raster,i,j).g);
                pixel_at_m(raster,i,j).b=(255-pixel_at_m(raster,i,j).b);
            }
    return;
}


void main_state_init(GLFWwindow *window, void *args, int width, int height)
{

    /* inicijalizacija */
    char buff[10];
    FILE *file;
    file = fopen("res/files/HighScore.txt", "r");
    fgets(buff, 10, (FILE*)file);
    fclose(file);
    //printf("%s %d",buff,strlen(buff));
    highScore=atoi(buff);
    for(int i=0;i<8;i++) ///random shuffle array
    {
        int j,tmp;
        j=rand()%(8-i)+i;
        tmp=randomArray[j];
        randomArray[j]=randomArray[i];
        randomArray[i]=tmp; // Swap i and j
    }
    rafgl_raster_load_from_image(&gameImage,"res/images/gameImage3.png");
    sprite.frame_height=gameImage.height;
    sprite.frame_width=gameImage.width;
    sprite.sheet_height=32;
    sprite.sheet_width=32;
    sprite.sheet=gameImage;
    rafgl_raster_load_from_image(&coinImage,"res/images/coins.png");
    spriteCoin.frame_height=coinImage.height;
    spriteCoin.frame_width=coinImage.width;
    spriteCoin.sheet_height=32;
    spriteCoin.sheet_width=32;
    spriteCoin.sheet=coinImage;
    rafgl_raster_load_from_image(&gameOverImage,"res/images/gameOver.png");
    spriteGameOver.frame_height=gameOverImage.height;
    spriteGameOver.frame_width=gameOverImage.width;
    spriteGameOver.sheet_height=gameOverImage.height;
    spriteGameOver.sheet_width=gameOverImage.width;
    spriteGameOver.sheet=gameOverImage;
    rafgl_raster_load_from_image(&highScoreImage,"res/images/newHS.png");
    spriteHighScore.frame_height=highScoreImage.height;
    spriteHighScore.frame_width=highScoreImage.width;
    spriteHighScore.sheet_height=highScoreImage.height;
    spriteHighScore.sheet_width=highScoreImage.width;
    spriteHighScore.sheet=highScoreImage;
    rafgl_raster_load_from_image(&mainPageImage,"res/images/mainPage.png");
    spriteMainPage.frame_height=mainPageImage.height;
    spriteMainPage.frame_width=mainPageImage.width;
    spriteMainPage.sheet_height=mainPageImage.height;
    spriteMainPage.sheet_width=mainPageImage.width;
    spriteMainPage.sheet=mainPageImage;
    rafgl_raster_load_from_image(&pauseImage,"res/images/pause1.png");
    spritePause.frame_height=pauseImage.height;
    spritePause.frame_width=pauseImage.width;
    spritePause.sheet_height=pauseImage.height;
    spritePause.sheet_width=pauseImage.width;
    spritePause.sheet=pauseImage;
    initButtons();
    srand(time(NULL));
    food.scale = 32;
    initSnake();
    spawnFruit();
    rafgl_raster_init(&raster, raster_width, raster_height);
    rafgl_raster_init(&raster2, raster_width, raster_height+SNAKE_OFFSET);
    //init_stars();
    rafgl_texture_init(&texture);

}

int pressed;
float location = 0;
float selector = 0;
float deltaTimeMove=0;
float deltaTimeCoin=0;
float deltaTimeFirework=0;

void updateRaster2()
{
    for(int j=0;j<raster2.height;j++)
    {
        for(int i=0;i<raster2.width;i++)
        {
            if(j<SNAKE_OFFSET)
            {
                if(currState==STATE_GAME || currState==STATE_PAUSE)
                    pixel_at_m(raster2,i,j).rgba=rafgl_RGB(255,255,204);
                else
                    pixel_at_m(raster2,i,j).rgba=rafgl_RGB(0,0,0);
            }
            else
            {
                //printf("%d %d dbg\n",i,j-SNAKE_OFFSET);
                pixel_at_m(raster2,i,j)=pixel_at_m(raster,i,j-SNAKE_OFFSET);
            }
        }
    }
}

void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args)
{
   //[ printf("%f\n",deltaTimeCheck);
     /* hendluj input */
    handleInput(game_data);
    deltaClickTime -= delta_time;
    if(currState==STATE_MAIN_MENU)
    {
        drawBackground();
        drawFromWholeSprite((raster_width-spriteMainPage.sheet_width)/2,50,spriteMainPage,0);
        drawButton(endlessBtn);
        if(checkHover(endlessBtn,game_data))
            drawHoverButton(endlessBtn);
        drawButton(quitBtn);
        if(checkHover(quitBtn,game_data))
            drawHoverButton(quitBtn);
        updateRaster2();
        rafgl_texture_load_from_raster(&texture, &raster2);
        return;
    }
    if(currState==STATE_PAUSE)
    {
        //drawFromWholeSprite((raster_width-spritePause.sheet_width)-15,10,spritePause,1);
        drawPause((raster_width-spritePause.sheet_width)-15,10,spritePause);
        //char strPause[3]={'|','|','\0'};
        //rafgl_raster_draw_string(&raster,strPause,raster_width-50,0,rafgl_RGB(255,255,255),2);
        updateRaster2();
        for(int i=0;i<currLevelProgress;i++)
        {
            //printf("%d lvl\n",currLevelProgress);
            switch(randomArray[i])
            {
                case 0: left_colour.rgba=COLOR1; break;
                case 1: left_colour.rgba=COLOR2; break;
                case 2: left_colour.rgba=COLOR3; break;
                case 3: left_colour.rgba=COLOR4; break;
                case 4: left_colour.rgba=COLOR5; break;
                case 5: left_colour.rgba=COLOR6; break;
                case 6: left_colour.rgba=COLOR7; break;
                case 7: left_colour.rgba=COLOR8; break;
            }
            switch(randomArray[i+1])
            {
                case 0: right_colour.rgba=COLOR1; break;
                case 1: right_colour.rgba=COLOR2; break;
                case 2: right_colour.rgba=COLOR3; break;
                case 3: right_colour.rgba=COLOR4; break;
                case 4: right_colour.rgba=COLOR5; break;
                case 5: right_colour.rgba=COLOR6; break;
                case 6: right_colour.rgba=COLOR7; break;
                case 7: right_colour.rgba=COLOR8; break;
            }
            float x_normalized;
            for(int y = 0; y < SNAKE_OFFSET; y++)
            {
                for(int x = i*(raster_width/8); x < i*(raster_width/8)+raster_width/8; x++)
                {
                    x_normalized = 1.0f * (x-i*(raster_width/8)) / (raster_width/8);
                    pixel_at_m(raster2, x, y) = lerp_pixel(left_colour, right_colour, x_normalized);
                }
            }
        }
        char strProgress[15]={'L','E','V','E','L',' ','P','R','O','G','R','E','S','S','\0'};
        rafgl_raster_draw_string(&raster2,strProgress,150,-20,rafgl_RGB(255,255,204),2);
        rafgl_texture_load_from_raster(&texture, &raster2);
        return;
    }
    if(currState==STATE_ENDGAME)
    {
        drawBackground();
        drawFromWholeSprite((raster_width-spriteGameOver.sheet_width)/2,50,spriteGameOver,0);
        drawButton(retryBtn);
        if(checkHover(retryBtn,game_data))
            drawHoverButton(retryBtn);
        drawButton(mainMenuBtn);
        if(checkHover(mainMenuBtn,game_data))
            drawHoverButton(mainMenuBtn);

        if(newHighScoreSet)
        {
            drawFromWholeSprite((raster_width-spriteHighScore.sheet_width)/2,50+spriteGameOver.sheet_height+12,spriteHighScore,0);
            deltaTimeFirework+=delta_time;
            if(deltaTimeFirework>=1){
                createFirework(delta_time);
                deltaTimeFirework = 0;
            }
            update_particles(delta_time);
            draw_particles(&raster);
        }
        updateRaster2();
        rafgl_texture_load_from_raster(&texture, &raster2);
        return;
    }
    /* izmeni raster */
    deltaTimeMove+=delta_time;
    deltaTimeCoin+=delta_time;
    if(deltaTimeMove>=deltaTimeCheck)
    {
        deltaTimeMove=0;
        inputLockFlag=0;
        updateSnake();
    }
    //printf(" debug %d %d\n",player.body[0].x, player.body[0].y);
    checkDeath();
    eat();
    drawBackground();
    if(score%5 || score==0)
    {
        changeColor=1;
        drawFood();
    }
    if(deltaTimeCoin>=0.07f && score%5==0 && score>0)
    {
        deltaTimeCoin=0;
        coinFrame++;
        coinFrame%=6;
    }
    if(score%5==0 && score>0)
    {
        changeColor=0;
        drawFromSpritesheet(food.posX,food.posY,food.scale,food.scale,spriteCoin,0,coinFrame);
    }
    drawSnake();
    char strScore[7]={'S','C','O','R','E',' ','\0'};
    char num[5];
    itoa(player.score,num,10);
    strcat(strScore,num);
    if(player.score<=highScore)
        rafgl_raster_draw_string(&raster,strScore,20,0,rafgl_RGB(255,255,255),1);
    else
        rafgl_raster_draw_string(&raster,strScore,20,0,rafgl_RGB(148,0,211),1);
    char strHighScore[12]={'H','I','G','H',' ','S','C','O','R','E',' ','\0'};
    itoa(highScore,num,10);
    strcat(strHighScore,num);
    rafgl_raster_draw_string(&raster,strHighScore,20,25,rafgl_RGB(255,255,255),1);
    char strLevel[7]={'L','E','V','E','L',' ','\0'};
    itoa(level,num,10);
    strcat(strLevel,num);
    rafgl_raster_draw_string(&raster,strLevel,20,50,rafgl_RGB(255,255,255),1);
    /* shift + s snima raster */
    if(game_data->keys_pressed[RAFGL_KEY_S] && game_data->keys_down[RAFGL_KEY_LEFT_SHIFT])
    {
        sprintf(save_file, "save%d.png", save_file_no++);
        rafgl_raster_save_to_png(&raster, save_file);
    }

    /*int star_speed = 5 + 90 * selector;

    update_stars(star_speed);
    render_stars(&raster, star_speed);*/
    /* update-uj teksturu*/
    updateRaster2();
    for(int i=0;i<currLevelProgress;i++)
    {
      //  printf("%d lvl\n",currLevelProgress);
        switch(randomArray[i])
        {
            case 0: left_colour.rgba=COLOR1; break;
            case 1: left_colour.rgba=COLOR2; break;
            case 2: left_colour.rgba=COLOR3; break;
            case 3: left_colour.rgba=COLOR4; break;
            case 4: left_colour.rgba=COLOR5; break;
            case 5: left_colour.rgba=COLOR6; break;
            case 6: left_colour.rgba=COLOR7; break;
            case 7: left_colour.rgba=COLOR8; break;
        }
        switch(randomArray[i+1])
        {
            case 0: right_colour.rgba=COLOR1; break;
            case 1: right_colour.rgba=COLOR2; break;
            case 2: right_colour.rgba=COLOR3; break;
            case 3: right_colour.rgba=COLOR4; break;
            case 4: right_colour.rgba=COLOR5; break;
            case 5: right_colour.rgba=COLOR6; break;
            case 6: right_colour.rgba=COLOR7; break;
            case 7: right_colour.rgba=COLOR8; break;
        }
        float x_normalized;
        for(int y = 0; y < SNAKE_OFFSET; y++)
        {
            for(int x = i*(raster_width/8); x < i*(raster_width/8)+raster_width/8; x++)
            {
                x_normalized = 1.0f * (x-i*(raster_width/8)) / (raster_width/8);
                pixel_at_m(raster2, x, y) = lerp_pixel(left_colour, right_colour, x_normalized);
            }
        }
    }
    char strProgress[15]={'L','E','V','E','L',' ','P','R','O','G','R','E','S','S','\0'};
    rafgl_raster_draw_string(&raster2,strProgress,150,-20,rafgl_RGB(255,255,204),2);
    rafgl_texture_load_from_raster(&texture, &raster2);
    if(game_data->keys_down[RAFGL_KEY_SPACE])
        rafgl_texture_load_from_raster(&texture, &spriteCoin.sheet);
}





void main_state_render(GLFWwindow *window, void *args)
{
    /* prikazi teksturu */
    rafgl_texture_show(&texture);

}


void main_state_cleanup(GLFWwindow *window, void *args)
{
    rafgl_raster_cleanup(&raster);
    rafgl_texture_cleanup(&texture);

}
