#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <ncurses/ncurses.h>
#include <string.h>

// for window

#define ROWS 20 // ตาราง
#define COLS 15
#define TRUE 1
#define FALSE 0

char Table[ROWS][COLS] = {0};
int score = 0;
char GameOn = FALSE;
useconds_t timer = 400000; // decrease this to make it faster
int decrease = 2190;
int selectShape = -1;
int cooldown = 10;
struct timeval before, now;
struct timeval timep2useskill;
char word[6]; // show cooldown

char player2interface[7][100] = {
    "       Long Bar key is 0",
    "       L shape key is 1",
    "       reverse L shape key is 2",
    "       square key is 3",
    "       S shape key is 4",
    "       Z shape key is 5",
    "       T shape key is 6"
};

// สร้างบล็อค
typedef struct {
    char **array;
    int width, row, col;
} Shape;
Shape current;

const Shape ShapesArray[7]= {
	{(char *[]){(char []){0,1,1},(char []){1,1,0}, (char []){0,0,0}}, 3},                           //S shape     
	{(char *[]){(char []){1,1,0},(char []){0,1,1}, (char []){0,0,0}}, 3},                           //Z shape     
	{(char *[]){(char []){0,1,0},(char []){1,1,1}, (char []){0,0,0}}, 3},                           //T shape     
	{(char *[]){(char []){0,0,1},(char []){1,1,1}, (char []){0,0,0}}, 3},                           //L shape     
	{(char *[]){(char []){1,0,0},(char []){1,1,1}, (char []){0,0,0}}, 3},                           //flipped L shape    
	{(char *[]){(char []){1,1},(char []){1,1}}, 2},                                                 //square shape
	{(char *[]){(char []){0,0,0,0}, (char []){1,1,1,1}, (char []){0,0,0,0}, (char []){0,0,0,0}}, 4} //long bar shape
	// ถ้าจะเพิ่มไปเปลี่ยนเลขอาเรย์ด้วย
};

bool CheckCoolDown(){
	gettimeofday(&now, NULL);
    long int diff = (now.tv_sec - timep2useskill.tv_sec);
	return cooldown <= (diff);
}

Shape CopyShape(Shape shape){
	Shape new_shape = shape;
	char **copyshape = shape.array;
	new_shape.array = (char**)malloc(new_shape.width*sizeof(char*));
    int i, j;
    for(i = 0; i < new_shape.width; i++){
		new_shape.array[i] = (char*)malloc(new_shape.width*sizeof(char));
		for(j=0; j < new_shape.width; j++) {
			new_shape.array[i][j] = copyshape[i][j];
		}
    }
    return new_shape;
}

void DeleteShape(Shape shape){
    int i;
    for(i = 0; i < shape.width; i++){
		free(shape.array[i]);
    }
    free(shape.array);
}

int CheckPosition(Shape shape){ //Check the position of the copied shape
	char **array = shape.array;
	int i, j;
	for(i = 0; i < shape.width;i++) {
		for(j = 0; j < shape.width ;j++){
			if((shape.col+j < 0 || shape.col+j >= COLS || shape.row+i >= ROWS)){ //Out of borders
				if(array[i][j]) //but is it just a phantom?
					return FALSE;
				
			}
			else if(Table[shape.row+i][shape.col+j] && array[i][j])
				return FALSE;
		}
	}
	return TRUE;
}

void RotateShape(Shape shape){ //ตามเข็ม
	Shape temp = CopyShape(shape);
	int i, j, k, width;
	width = shape.width;
	for(i = 0; i < width ; i++){
		for(j = 0, k = width-1; j < width ; j++, k--){
				shape.array[i][j] = temp.array[k][i];
		}
	}
	DeleteShape(temp);
}

void RotateReverse(Shape shape){ // ทวนเข็ม
    Shape temp = CopyShape(shape);
    int i, j, k, width;
    width = shape.width;
    for (i = 0; i < width; i++) {
        for(j = 0, k = width-1; j < width ; j++, k--){
				shape.array[j][i] = temp.array[i][k];
        }
    }
	DeleteShape(temp);
}

void SetNewRandomShape(){ //updates [current] with new shape
	Shape new_shape;
	if(selectShape > -1){
		new_shape = CopyShape(ShapesArray[selectShape]);
		selectShape = -1;
	}else{
		new_shape = CopyShape(ShapesArray[rand()%7]);
	}

    //สุ่มอีกทีตอนเกิด
    int selSpawn = rand()%4;
    if (selSpawn == 0){
        RotateShape(new_shape);
    }
    else if(selSpawn == 1){
        RotateReverse(new_shape);
    }
    else if(selSpawn == 2){
        RotateShape(new_shape);
        RotateShape(new_shape);
    }

    new_shape.col = rand()%(COLS-new_shape.width+1);
    new_shape.row = 0;
    DeleteShape(current);
	current = new_shape;
	if(!CheckPosition(current)){
		GameOn = FALSE;
	}
}

void WriteToTable(){
	int i, j;
	for(i = 0; i < current.width ;i++){
		for(j = 0; j < current.width ; j++){
			if(current.array[i][j])
				Table[current.row+i][current.col+j] = current.array[i][j];
		}
	}
}

void RemoveFullRowsAndUpdateScore(){
	int i, j, sum, count=0;
	for(i=0;i<ROWS;i++){
		sum = 0;
		for(j=0;j< COLS;j++) {
			sum+=Table[i][j];
		}
		if(sum==COLS){
			count++;
			int l, k;
			for(k = i;k >=1;k--)
				for(l=0;l<COLS;l++)
					Table[k][l]=Table[k-1][l];
			for(l=0;l<COLS;l++)
				Table[k][l]=0;
			timer-=decrease--;
		}
	}
	score += 219*count;
}

void PrintTable(){
	char Buffer[ROWS][COLS] = {0};
	int i, j;
	for(i = 0; i < current.width ;i++){
		for(j = 0; j < current.width ; j++){
			if(current.array[i][j])
				Buffer[current.row+i][current.col+j] = current.array[i][j];
		}
	}
	clear();
	for(i=0; i<COLS-9; i++)
		printw(" ");
	printw("Tetris Game\n");

	
	for(i = 0; i < ROWS ;i++){
		for(j = 0; j < COLS ; j++){
			attron(COLOR_PAIR(1));
			printw("%c ", (Table[i][j] + Buffer[i][j])? 219: '.');
			attroff(COLOR_PAIR(1));
		}
        if(i>=2 && i<=8){
            printw("%s", player2interface[i-2]);
        }
		printw("\n");
	}
	
	printw("\nScore: %d\n", score);
	printw("\nCooldown: ");
	if(CheckCoolDown()){
		attron(COLOR_PAIR(3));
		printw(" Ready \n");
		attroff(COLOR_PAIR(3));
	}else{
		attron(COLOR_PAIR(2));
		printw(" Wait \n");
		attroff(COLOR_PAIR(2));
	}
	
	
}

void ManipulateCurrent(int action){
	Shape temp = CopyShape(current);
	switch(action){
		case 's':
			temp.row++;  //move down
			if(CheckPosition(temp))
				current.row++;
			else {
				WriteToTable();
				RemoveFullRowsAndUpdateScore();
                SetNewRandomShape();
			}
			break;
		case 'd':
			temp.col++;  //move right
			if(CheckPosition(temp))
				current.col++;
			break;
		case 'a':
			temp.col--;  //move left
			if(CheckPosition(temp))
				current.col--;
			break;
		case 'w':
			RotateShape(temp); // ตามเข็ม
			if(CheckPosition(temp))
				RotateShape(current);
			break;
        case 'e':
            RotateReverse(temp); // ทวนเข็ม
            if(CheckPosition(temp))
				RotateReverse(current);
			break;
        case ' ': // Spacebar
            while (CheckPosition(temp)) {
                current.row++;
                temp.row++;
            }
            current.row--;
            WriteToTable();
            RemoveFullRowsAndUpdateScore();
            SetNewRandomShape();
            break;
		case 'y':
			GameOn = TRUE;
			break;
		case 27 :
			GameOn = FALSE;
			break;
	}
	if(CheckCoolDown()){
		switch(action){
			case '4':
				selectShape = 0;
				gettimeofday(&timep2useskill, NULL);
				break;
			case '5':
				selectShape = 1;
				gettimeofday(&timep2useskill, NULL);
				break;
			case '6':
				selectShape = 2;
				gettimeofday(&timep2useskill, NULL);
				break;
			case '1':
				selectShape = 3;
				gettimeofday(&timep2useskill, NULL);
				break;
			case '2':
				selectShape = 4;
				gettimeofday(&timep2useskill, NULL);
				break;
			case '3':
				selectShape = 5;
				gettimeofday(&timep2useskill, NULL);
				break;
			case '0':
				selectShape = 6;
				gettimeofday(&timep2useskill, NULL);
				break;
		}
	}
	
	DeleteShape(temp);
	PrintTable();
}

//use before and now
int hasToUpdate(){
	return ((useconds_t)(now.tv_sec*2190000 + now.tv_usec) -((useconds_t)(before.tv_sec*2190000 + before.tv_usec))) > timer;
}

bool Home(){
	int c;
	printw("================================================\n");
	printw("		TetrisGame		\n");
	printw("================================================\n\n\n");
	
	attron(COLOR_PAIR(3));printw(" Start Game[Enter : Y] \n");attroff(COLOR_PAIR(3));
	attron(COLOR_PAIR(2));printw(" Quit Game[Enter : ESC] \n");attroff(COLOR_PAIR(2));

	while (TRUE)
	{
		c = getch();
		if(c == 27){
			return true;
		}
		else if (c != ERR) {
		  ManipulateCurrent(c);
		}
		if(GameOn == TRUE){
			return false;
		}
	}
	
	
}

int main() {
    srand(time(0));
    score = 0;
    int c;
    initscr();// ncurses start
	start_color();//สี
	init_pair(1, COLOR_BLACK, COLOR_WHITE);//table
	init_pair(2, COLOR_WHITE, COLOR_RED);//ERR or Quit or wait
	init_pair(3, COLOR_WHITE, COLOR_GREEN);// complete
	if(Home()){
		return 0;
	}
	gettimeofday(&before, NULL);
	timeout(1);
	SetNewRandomShape();
    PrintTable();
	gettimeofday(&timep2useskill, NULL);
	while(GameOn){
		if ((c = getch()) != ERR) {
		  ManipulateCurrent(c);
		}
		gettimeofday(&now, NULL);
		if (hasToUpdate()) { //time difference in microsec accuracy
			ManipulateCurrent('s');
			gettimeofday(&before, NULL);
		}
	}
	DeleteShape(current);
	endwin();
	int i, j;
	for(i = 0; i < ROWS ;i++){
		for(j = 0; j < COLS ; j++){
			printf("%c ", Table[i][j] ? 219: '.');
		}
		printf("\n");
	}
	printf("\nGame over!\n");
	printf("\nScore: %d\n", score);
    return 0;
}