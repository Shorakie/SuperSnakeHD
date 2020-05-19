#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <windows.h>

#define table(X,Y,WIDTH) table[(Y)*WIDTH + X]

//#define HIGHSCORE_FILE_PATH "C:/scores.hsf"
#define LEVEL_FILE_PATH "./%d.lvl"

typedef struct {
    int x;
    int y;
} Pos;

typedef struct _Node Node;

struct _Node{
    Pos pos;
    int direction;
    Node* prev;
}; // Linked List :))

typedef struct {
    int len;
    Node* head;
    Node* tail;
} Snake;

typedef Pos Hole;
typedef struct {
    Pos pos;
    int dir;
} Hedgehog;

typedef struct {
    int width;
    int height;
    int* table;
} Table;

typedef struct {
    int eggGoal;
    int hedgehogs;
    Table levelTable;
} Level;

/*typedef struct {
    char* name;
    int score;
} PlayerScore;*/

typedef struct {
    Pos pos;
    int time;
} Food;

enum Color {
	FG_BLACK		= 0x00,
	FG_DARK_BLUE    = 0x01,
	FG_DARK_GREEN   = 0x02,
	FG_DARK_CYAN    = 0x03,
	FG_DARK_RED     = 0x04,
	FG_DARK_MAGENTA = 0x05,
	FG_DARK_YELLOW  = 0x06,
	FG_GREY			= 0x07,
	FG_DARK_GREY    = 0x08,
	FG_BLUE			= 0x09,
	FG_GREEN		= 0x0A,
	FG_CYAN			= 0x0B,
	FG_RED			= 0x0C,
	FG_MAGENTA		= 0x0D,
	FG_YELLOW		= 0x0E,
	FG_WHITE		= 0x0F,
	BG_BLACK		= 0x00,
	BG_DARK_BLUE	= 0x10,
	BG_DARK_GREEN	= 0x20,
	BG_DARK_CYAN	= 0x30,
	BG_DARK_RED		= 0x40,
	BG_DARK_MAGENTA = 0x50,
	BG_DARK_YELLOW	= 0x60,
	BG_GREY			= 0x70,
	BG_DARK_GREY	= 0x80,
	BG_BLUE			= 0x90,
	BG_GREEN		= 0xA0,
	BG_CYAN			= 0xB0,
	BG_RED			= 0xC0,
	BG_MAGENTA		= 0xD0,
	BG_YELLOW		= 0xE0,
	BG_WHITE		= 0xF0,
};

enum States {
    STATE_MENU,
    STATE_PLAY,
    STATE_GAMEOVER,
    STATE_SCOREBOARD,
};

enum Mode{
    MODE_PLAYING,
    MODE_PUASE,
    MODE_IDLE,
    MODE_DEAD,
};

enum Scores{
    SCORE_STAR = 100,
    SCORE_EGG = 50,
    SCORE_LEVEL_COMPLETION = 150,
    SCORE_FOOD = 250,
};

enum Keys {
    K_UP = 72,
    K_RIGHT = 77,
    K_DOWN = 80,
    K_LEFT = 75,
    K_ENTER = 13,
    K_P = 112,
};

enum Dir {
    DIR_STOP,
    DIR_UP,
    DIR_DOWN,
    DIR_RIGHT,
    DIR_LEFT,
};

enum Objects {
    O_GRASS,
    O_WALL,
    O_SNAKE,
    O_EGG,
    O_STAR,
    O_HEDGEHOG,
    O_SNAKE_BODY,
    O_HOLE_ENABLE,
    O_HOLE_DISABLE,
    O_FOOD,
};

void setup();
void setupLevel();
void setupSnake();
void input();

void updateMenu();
void updateGame();
void updateGameOver();
void updateScoreboard();

void drawMenu();
void drawGame();
void drawGameOver();
void drawScoreboard();

void drawStatistics();
void drawMessage(const char* message);
void showScoreRewind();

void drawTable(const Table table);

void clearScreen();
void setColor(const int color);
int rndNum(int min, int max); // Includes Both min max
void freeSnake(Snake* snake);
void freeHedgehogs();
int getMaxEggsInScreen(int goal);
int getAheadTile(Pos pos, int dir);
Pos getAheadPos(Pos pos, int dir);

void readLevel(const char* path, Level* level);
//PlayerScore* readHighscores(int* count);
//void saveHighscore(const char* name, int score);
Pos placeObjectRandomly(int id, Table table);

void attachToTail(Node* node);
void moveSnake();
void eatAheadTile();
void collisionSnake(int collidingWith);
void damageSnake();

void drawFood();

void moveHedgehog(Hedgehog* hedgehog);
void collisionHedgehog(Hedgehog* hedgehog, int colldingWith);

int key;
Level currentLevel;
Table drawingTable;
int state = STATE_MENU;
int mode = MODE_IDLE;
int color[] = {
    BG_GREEN|FG_GREEN,          // GRASS
    FG_DARK_YELLOW|BG_GREEN,    // WALL
    FG_RED | BG_GREEN,          // Snake Head
    FG_WHITE | BG_GREEN,        // Egg
    FG_YELLOW | BG_GREEN,       // Star
    FG_DARK_MAGENTA | BG_GREEN, // Hedgehog
    FG_DARK_RED | BG_GREEN,     // Snake body
    FG_BLACK | BG_GREEN,        // HOLE Enabled
    FG_DARK_GREY | BG_GREEN,    // HOLE Disabled
    FG_DARK_GREEN | BG_GREEN,   // FOOD
};

char ascii[] = {' ', 178,'O','@','*','#','o','@','@', '$'};
int maxEggInScreen = 1;

// Statistics
int levelNumber = 1;
int score = 0;
int eatenEggs = 0;
int eatenStars = 0;

// Objects
Snake snake;
Hedgehog* hedgehogs = 0;
Hole hole;
Food food;
char* msg = NULL;

/// Menu
int selected = 0;

int main() {
    srand(time(NULL));
    setup();
    while(1){
        input();
        if(state == STATE_PLAY) {
            updateGame();
            drawGame();
        }else if(state == STATE_GAMEOVER){
            updateGameOver();
            drawGameOver();
        } else if(state == STATE_MENU){
            updateMenu();
            drawMenu();
        } else if(state == STATE_SCOREBOARD){
            updateScoreboard();
            drawScoreboard();
        }
        //Sleep(50);
        memcpy(drawingTable.table, currentLevel.levelTable.table, drawingTable.width * drawingTable.height * sizeof(int));
        clearScreen();
    }
    return 0;
}

void setup(){
    food.pos = (Pos){-1,-1};
    food.time = -1;
    setupLevel();
}

void setupLevel(){
    char path[15];
    sprintf(path, LEVEL_FILE_PATH, levelNumber);
    readLevel(path, &currentLevel);
    int i;

    // Setup Eggs
    maxEggInScreen=getMaxEggsInScreen(currentLevel.eggGoal);
    for(i = 0; i < maxEggInScreen; i++){
        placeObjectRandomly(O_EGG, currentLevel.levelTable);
    }

    // Setup Stars
    for(i = 0; i < 3; i++)
        placeObjectRandomly(O_STAR, currentLevel.levelTable);

    // Setup hole
    hole = placeObjectRandomly(O_HOLE_DISABLE, currentLevel.levelTable);

    drawingTable = currentLevel.levelTable;
    drawingTable.table = (int*) malloc(drawingTable.width * drawingTable.height * sizeof(int));
    memcpy(drawingTable.table, currentLevel.levelTable.table, drawingTable.width * drawingTable.height * sizeof(int));

    // Setup Hedgehogs
    hedgehogs = (Hedgehog*)malloc(currentLevel.hedgehogs * sizeof(Hedgehog));
    for(i = 0; i < currentLevel.hedgehogs; i++)
        hedgehogs[i] = (Hedgehog){placeObjectRandomly(O_HEDGEHOG, drawingTable), rndNum(DIR_RIGHT, DIR_LEFT)};

    setupSnake();

    mode = MODE_IDLE;

}

void setupSnake(){
    // Make a node as head and tail for the start
    snake.len = 1;

    Node* node = (Node*) malloc(1*sizeof(Node));
    node->direction = DIR_STOP;
    node->prev = NULL;
    node->pos = hole;
    drawingTable.table(hole.x,hole.y, drawingTable.width) = O_SNAKE;

    snake.head = node;
    snake.tail = node;
}

void clearScreen(){
    // Assuming GetStdHandle always retrieves a valid handle
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD){0,0});
}

void input(){
    if(kbhit()){
        key = getch();
        if(key == 224) // If it is one of arrow keys omit the 224
            key = getch();
    }
}

void updateMenu() {
    if(key == K_UP && selected > 0)
        selected--;
    else if(key == K_DOWN && selected < 2)
        selected++;

    if(key == K_ENTER){
        if(selected == 0)
            state = STATE_PLAY;
        else if(selected == 1)
            ;//state = STATE_SCOREBOARD;
        else if(selected == 2)
            exit(-1);
    }

    key = 0;
}

void updateGame() {
    if(mode == MODE_PUASE && key == K_P){
        key = 0;
        mode = MODE_PLAYING;
    }else if((mode == MODE_PLAYING || mode == MODE_IDLE) && key == K_P){
        key = 0;
        mode = MODE_PUASE;
    }
    if(mode == MODE_PUASE)
        return;

    // Update Snake Head direction
    if(key == K_UP && (snake.head->direction != DIR_DOWN || snake.len == 1)){
        snake.head->direction = DIR_UP;
        mode = MODE_PLAYING;
    }else if(key == K_RIGHT && (snake.head->direction != DIR_LEFT || snake.len == 1)){
        snake.head->direction = DIR_RIGHT;
        mode = MODE_PLAYING;
    }else if(key == K_DOWN && (snake.head->direction != DIR_UP || snake.len == 1)){
        snake.head->direction = DIR_DOWN;
        mode = MODE_PLAYING;
    }else if(key == K_LEFT && (snake.head->direction != DIR_RIGHT || snake.len == 1)){
        snake.head->direction = DIR_LEFT;
        mode = MODE_PLAYING;
    }
    // Tick food
    if(food.time != -1){
        food.time--;
        if(food.time == -1) food.pos = (Pos){-1,-1};
    }
    // Spawn food
    if(rndNum(1,60)<=1 && food.time == -1 && mode == MODE_PLAYING){
        food.time = 21;
        food.pos = placeObjectRandomly(O_FOOD, drawingTable);
    }

    collisionSnake(getAheadTile(snake.head->pos, snake.head->direction));
    if(snake.head->direction != DIR_STOP)
        moveSnake();

    //updateSnakeDirection(&snake);
    int i;
    for(i = 0; i < currentLevel.hedgehogs; i++){
        collisionHedgehog(&hedgehogs[i], getAheadTile(hedgehogs[i].pos, hedgehogs[i].dir));
        if(getAheadTile(hedgehogs[i].pos, hedgehogs[i].dir) == O_GRASS)
            moveHedgehog(&hedgehogs[i]);
    }
}

void updateGameOver(){}
void updateScoreboard() {}

void drawMenu(){
    setColor(BG_BLACK|FG_WHITE);
    printf("SUPER SNAKE HD\n\n");
    if(selected == 0) setColor(BG_RED|FG_WHITE); else setColor(BG_BLACK|FG_WHITE);
    printf("PLAY\n");
    if(selected == 1) setColor(BG_RED|FG_WHITE); else setColor(BG_BLACK|FG_WHITE);
    printf("SCORE BOARD\n");
    if(selected == 2) setColor(BG_RED|FG_WHITE); else setColor(BG_BLACK|FG_WHITE);
    printf("EXIT\n");

    if(state != STATE_MENU){
        setColor(BG_BLACK|FG_WHITE);
        system("cls");
    }

}

void drawSnake(){
    int i;
    Node* nodeptr = snake.tail;
    drawingTable.table(snake.head->pos.x,snake.head->pos.y, drawingTable.width) = O_SNAKE;
    for(i = 0; i < snake.len - 1; i++){
        drawingTable.table[nodeptr->pos.y * drawingTable.width + nodeptr->pos.x] = O_SNAKE_BODY;
        nodeptr = nodeptr->prev;
    }
}

void drawHedgehog(Hedgehog* hedgehog){
    drawingTable.table(hedgehog->pos.x, hedgehog->pos.y, drawingTable.width) = O_HEDGEHOG;
}

void drawGame(){
    drawSnake();

    int i;
    for(i = 0; i < currentLevel.hedgehogs; i++)
        drawHedgehog(&hedgehogs[i]);

    if(msg)
        drawMessage(msg);
    if(food.time != -1)
        drawFood();
    drawTable(drawingTable);
    if(msg) {Sleep(800); msg = NULL;}
    drawStatistics();
}

void drawGameOver(){
    setColor(BG_BLACK|FG_WHITE);
    system("cls");

    setColor(BG_BLACK|FG_RED);
    printf("\n\n\t\tGAME OVER!!");
    Sleep(1000);

    setColor(BG_BLACK|FG_WHITE);
    system("cls");

    showScoreRewind();
    printf("\n\n        ");
    setColor(BG_BLACK|FG_WHITE);
    printf("Press any key to continue");
    getch();

    eatenEggs = 0;
    key = 0;
    // Free Snake nods from memory
    freeSnake(&snake);
    // Free hedgehogs from memory
    freeHedgehogs();

    food.time = -1;
    food.pos = (Pos){-1,-1};

    score = 0;
    levelNumber = 1;
    state = STATE_MENU;
    system("cls");
    setupLevel();
}

void drawScoreboard(){
    /*int count, i;
    PlayerScore* scores = readHighscores(&count);
    for(i = 0; i< count; i++){
        printf("\t\t%s:\t%d\n", scores[i].name,scores[i].score);
    }
    getch();*/
}

void drawStatistics(){
    setColor(FG_WHITE | BG_BLACK); printf("Level: "); setColor(FG_RED | BG_BLACK); printf("%d\t",levelNumber);

    setColor(FG_WHITE | BG_BLACK); printf("[");
    int i;
    for(i = 0; i < 3; i++){
        if(eatenStars > i){
            setColor(FG_YELLOW | BG_BLACK); printf("*");
        }else{
            setColor(FG_WHITE | BG_BLACK); printf(" ");
        }
        setColor(FG_WHITE | BG_BLACK);
        if(i == 2)
            printf("]\t");
        else
            printf("][");
    }

    setColor(FG_WHITE | BG_BLACK); printf("Eggs (@): "); setColor(FG_MAGENTA | BG_BLACK); printf("%d", eatenEggs);
    printf("/");
    setColor(FG_DARK_MAGENTA | BG_BLACK); printf("%d\t\t", currentLevel.eggGoal);

    setColor(FG_WHITE | BG_BLACK); printf("Score : "); setColor(FG_YELLOW | BG_BLACK); printf("%d\t",score);
}

void drawMessage(const char* message){
    int msgLen = strlen(message);
    int startX = snake.head->pos.x - msgLen/2;
    if(startX < 0)
        startX = 0;
    if(startX + msgLen + 1 > drawingTable.width)
        startX -= startX + msgLen - drawingTable.width;
    int startY = snake.head->pos.y - 1;
    int i;
    for(i = 0; i < msgLen; i++){
        drawingTable.table(startX+i, startY, drawingTable.width) = message[i];
    }
}


void showScoreRewind(){

        system("cls");
        setColor(BG_BLACK|FG_WHITE);
        printf("\n\n\n\t\t");int i;
        for(i = 0; i < 3; i++){
            if(eatenStars > i){
                setColor(FG_YELLOW | BG_BLACK); printf("* ");
            }else{
                setColor(FG_WHITE | BG_BLACK); printf("[] ");
            }
        }
        printf("\n\t\t");
        setColor(FG_WHITE | BG_BLACK); printf("Level: "); setColor(FG_RED | BG_BLACK); printf("%d\n",levelNumber);

        printf("\n\n\t       ");
        setColor(FG_WHITE | BG_BLACK); printf("Score : "); setColor(FG_YELLOW | BG_BLACK); printf("%d\t",score);
}

void drawTable(const Table table){
    int i,j;
    for(j = 0; j < table.height; j++){
        for(i = 0; i < table.width ; i++){
            int id = table.table[j * table.width + i];
            if(id < 32){
                setColor(color[id]);
                printf("%c", ascii[id]);
            }else{
                setColor(BG_WHITE|FG_BLUE);
                printf("%c", id);
            }
        }
        printf("\n");
    }
}

void setColor(const int color){
    // Assuming GetStdHandle always retrieves a valid handle
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

int rndNum(int min, int max){
    return rand()%(max - min + 1) + min;
}

void freeSnake(Snake* snake){
    int i;
    Node* nodeptr = snake->tail;
    for(i = 0; i < snake->len; i++){
        free(nodeptr);
        nodeptr = nodeptr->prev;
    }
}

void freeHedgehogs(){
    free(hedgehogs);
}

int getMaxEggsInScreen(int goal){
    int max = goal/3;
    if(max >= 1 && max <= 5)
        return max;
    else if (max > 5)
        return 5;
    else if (max < 1)
        return 1;
}

int getAheadTile(Pos pos, int dir){
    if(dir == DIR_UP)
        return drawingTable.table[(pos.y-1) * drawingTable.width + pos.x];
    else if(dir == DIR_RIGHT)
        return drawingTable.table[pos.y * drawingTable.width + (pos.x+1)];
    else if(dir == DIR_DOWN)
        return drawingTable.table[(pos.y+1) * drawingTable.width + pos.x];
    else if(dir == DIR_LEFT)
        return drawingTable.table[pos.y * drawingTable.width + (pos.x-1)];
    else if(dir == DIR_STOP)
        return drawingTable.table[pos.y * drawingTable.width + pos.x];
}

Pos getAheadPos(Pos pos, int dir){
    Pos res = pos;
    if(dir == DIR_UP)
        res.y--;
    else if(dir == DIR_RIGHT)
        res.x++;
    else if(dir == DIR_DOWN)
        res.y++;
    else if(dir == DIR_LEFT)
        res.x--;
    return res;
}
void eatAheadTile(){
    int dir = snake.head->direction;
    Pos pos = snake.head->pos;
    if(dir == DIR_UP)
        currentLevel.levelTable.table[(pos.y-1) * currentLevel.levelTable.width + pos.x] = O_GRASS;
    else if(dir == DIR_RIGHT)
        currentLevel.levelTable.table[pos.y * currentLevel.levelTable.width + (pos.x+1)] = O_GRASS;
    else if(dir == DIR_DOWN)
        currentLevel.levelTable.table[(pos.y+1) * currentLevel.levelTable.width + pos.x] = O_GRASS;
    else if(dir == DIR_LEFT)
        currentLevel.levelTable.table[pos.y * currentLevel.levelTable.width + (pos.x-1)] = O_GRASS;
}

void readLevel(const char* path, Level* level){
    FILE* levelFile = fopen(path, "r");

    // If file doesn't exist
    if(levelFile == NULL){
        printf("Couldn't Find level file: %s\nClosing...", path);
        exit(-1);
    }

    // Read header
    fscanf(levelFile, "%d%d%d%d", &level->eggGoal, &level->hedgehogs,&level->levelTable.width, &level->levelTable.height);

    //Make a buffer for the table
    int totalTiles = level->levelTable.width * level->levelTable.height;
    level->levelTable.table = (int*)malloc(totalTiles * sizeof(int));
    int i;
    for(i = 0; i < totalTiles; i++)
        fscanf(levelFile, "%1d", &level->levelTable.table[i]);

    fclose(levelFile);
}

/*PlayerScore* readHighscores(int* count){
    FILE* highscoreFile = fopen(HIGHSCORE_FILE_PATH, "r");
    *count = 0;
    int playerScore = 0;
    char playerName[15];
    fscanf(highscoreFile,"%d\n", count);
    PlayerScore* res = (PlayerScore*)malloc(*count * sizeof(PlayerScore));
    int i;
    for(i = 0; i < *count; i++){
        if(fscanf(highscoreFile, "%s\t%d\n", res[i].name, &res[i].score) == EOF)
            break;
    }
    return res;
}

void saveHighscore(const char* name, int score){
    FILE* highscoreFile = fopen(HIGHSCORE_FILE_PATH, "r+");

}*/

Pos placeObjectRandomly(int id, Table table){
    int x = 1;
    int y = 1;
    do{
        x = rndNum(1, table.width - 1);
        y = rndNum(1, table.height - 1);
    } while (table.table[y * table.width + x] != O_GRASS);
    table.table[y * table.width + x] = id;
    return (Pos){x,y};
}

void moveSnake(){
    int i;
    Node* nodeptr = snake.tail;
    for(i = 0; i < snake.len; i++){
        if(nodeptr->direction == DIR_UP)
            nodeptr->pos.y--;
        else if(nodeptr->direction == DIR_RIGHT)
            nodeptr->pos.x++;
        else if(nodeptr->direction == DIR_DOWN)
            nodeptr->pos.y++;
        else if(nodeptr->direction == DIR_LEFT)
            nodeptr->pos.x--;

        if(nodeptr->prev != NULL)   // Update the Direction
            nodeptr->direction = nodeptr->prev->direction;
        nodeptr = nodeptr->prev;
    }
}

void moveHedgehog(Hedgehog* hedgehog){
    if(hedgehog->dir == DIR_UP)
        hedgehog->pos.y--;
    else if(hedgehog->dir == DIR_RIGHT)
        hedgehog->pos.x++;
    else if(hedgehog->dir == DIR_DOWN)
        hedgehog->pos.y++;
    else if(hedgehog->dir == DIR_LEFT)
        hedgehog->pos.x--;
}

void collisionHedgehog(Hedgehog* hedgehog, int collidingWith){
    if(collidingWith == O_WALL || collidingWith == O_EGG || collidingWith == O_STAR){
        // Revert movement
        if(hedgehog->dir == DIR_LEFT)
            hedgehog->dir = DIR_RIGHT;
        else if(hedgehog->dir == DIR_RIGHT)
            hedgehog->dir = DIR_LEFT;
        return;
    }

    Pos headingPos = getAheadPos(hedgehog->pos, hedgehog->dir);
    // Snake head
    if(snake.head->pos.x == headingPos.x && snake.head->pos.y == headingPos.y){
        damageSnake();
        *hedgehog = (Hedgehog){placeObjectRandomly(O_HEDGEHOG, drawingTable), rndNum(DIR_RIGHT, DIR_LEFT)}; // Place new Hedgehog
        return;
    }
    // Snake body
    int i;
    Node* nodeptr = snake.tail;
    for(i = 0; i < snake.len - 1; i++){
        if(nodeptr->pos.x == headingPos.x && nodeptr->pos.y == headingPos.y){
            // Revert movement
            if(hedgehog->dir == DIR_LEFT)
                hedgehog->dir = DIR_RIGHT;
            else if(hedgehog->dir == DIR_RIGHT)
                hedgehog->dir = DIR_LEFT;
            return;
        }
        nodeptr = nodeptr->prev;
    }
}

void attachToTail(Node* node){
    snake.len++;
    node->pos = snake.tail->pos;
    node->direction = DIR_STOP;
    node->prev = snake.tail;
    snake.tail = node;
}

void damageSnake(){
    msg = "OUCH!";
    if(eatenStars <= 0){
        state = STATE_GAMEOVER;
        mode = MODE_DEAD;
        snake.head->direction = DIR_STOP;
    }else{
        eatenStars--;
    }
}

void collisionSnake(int collidingWith){
    if(collidingWith == O_WALL){ // Wall collisions
        snake.head->direction = DIR_STOP;
        mode = MODE_IDLE;
        damageSnake();
        key = 0;
        return;
    }else if(collidingWith == O_STAR){ // Star collision
        if(eatenStars < 3)
            eatenStars++;
        score += SCORE_STAR;
        eatAheadTile();
        return;
    }else if(collidingWith == O_EGG) {  // Egg collision
        eatenEggs++;
        score += SCORE_EGG;
        eatAheadTile();

        if(eatenEggs >= currentLevel.eggGoal) // Enable the gate to next level
            currentLevel.levelTable.table(hole.x, hole.y, currentLevel.levelTable.width) = O_HOLE_ENABLE;

        placeObjectRandomly(O_EGG, currentLevel.levelTable);
        attachToTail(malloc(sizeof(Node)));
        return;
    }else if(collidingWith == O_HOLE_ENABLE){   // Go to the next level
        levelNumber++;
        score += SCORE_LEVEL_COMPLETION;
        eatenEggs = 0;
        key = 0;
        // Free Snake nods from memory
        freeSnake(&snake);
        // Free hedgehogs from memory
        freeHedgehogs();

        food.time = -1;
        food.pos = (Pos){-1,-1};

        showScoreRewind();
        printf("\n\n        ");
        setColor(BG_BLACK|FG_WHITE);
        printf("Press any key to continue");
        getch();

        setupLevel();
        system("cls");
        return;
    }
    Pos headingPos = getAheadPos(snake.head->pos, snake.head->direction);
    if(headingPos.x == food.pos.x && headingPos.y == food.pos.y){
        food.time = -1;
        food.pos = (Pos){-1,-1};
        score+=SCORE_FOOD;
    }

    int i;
    // Snake body collision
    Node* nodeptr = snake.tail;
    for(i = 0; i < snake.len - 1; i++){
        if(nodeptr->pos.x == headingPos.x && nodeptr->pos.y == headingPos.y){
            snake.head->direction = DIR_STOP;
            mode = MODE_IDLE;
            damageSnake();
            key = 0;
            return;
        }
        nodeptr = nodeptr->prev;
    }

    // Hedgehog Collision
    for(i = 0; i < currentLevel.hedgehogs; i++){

        if(hedgehogs[i].pos.x == headingPos.x && hedgehogs[i].pos.y == headingPos.y){
            damageSnake();
            hedgehogs[i] = (Hedgehog){placeObjectRandomly(O_HEDGEHOG, drawingTable), rndNum(DIR_RIGHT, DIR_LEFT)};
            return;
        }
    }
}

void drawFood(){
    if((food.time < 5 && food.time%2==1) ||  food.time>=5){
        drawingTable.table(food.pos.x,food.pos.y, drawingTable.width) = O_FOOD;
    }
}
