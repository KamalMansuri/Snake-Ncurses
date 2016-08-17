#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum Directions {UP, DOWN, LEFT, RIGHT, HOLD};
enum Option {OPEN, CLOSED};
enum Status {GAME_ON, GAME_OVER};

struct LinkedItem {
	char symbol;
	int x, y;
	time_t active_time;
	enum Directions direction;
	struct LinkedItem *next;
};

typedef struct {
	struct LinkedItem *snake;
	struct LinkedItem *food;
	struct LinkedItem *bonus_food;
	int max_x, max_y;
	int food_until_bonus;
	int long score;
	enum Option option;
}Board;

struct LinkedItem createCell(int x, int y, char symbol) {
	struct LinkedItem cell;
	cell.x = x; cell.y = y;
	cell.symbol = symbol;

	return cell;
}

struct LinkedItem *createSnake() {
	struct LinkedItem *head    = malloc(sizeof(struct LinkedItem));
	struct LinkedItem *middle1 = malloc(sizeof(struct LinkedItem));
	struct LinkedItem *middle2 = malloc(sizeof(struct LinkedItem));
	struct LinkedItem *tail    = malloc(sizeof(struct LinkedItem));

	*head    = createCell(1,4,'v'); 
	*middle1 = createCell(1,3,'o');
	*middle2 = createCell(1,2,'o');
	*tail    = createCell(1,1,'o');

	head -> direction    = DOWN;
	head -> next         = middle1;
	middle1 -> direction = DOWN;
	middle1 -> next      = middle2;
	middle2 -> direction = DOWN;
	middle2 -> next 		 = tail;
	tail -> direction 	 = DOWN;
	tail -> next         = NULL;

	return head;
}

bool inSamePlace(struct LinkedItem *new_item, struct LinkedItem *snake) {
	struct LinkedItem *head = snake;

	while(head) {
		if( (new_item -> x == head -> x) && (new_item -> y == head -> y) ) {
			return true;
		}
		head = head -> next;
	}
	return false;
}

/* function obtained online from stackoverflow */
int randomAtMost(int min ,int max) {
	int r;

	r = max + rand() / (RAND_MAX / (min - max + 1) + 1);

	return r;
}

void addFood(Board *board) {
	int new_x = randomAtMost(1, board -> max_x - 3); int new_y = randomAtMost(1, board -> max_y - 3);
	struct LinkedItem *food = board -> food;
	food -> x = new_x; food -> y = new_y;

	while( inSamePlace(food, board -> snake) ) {
		food -> x = randomAtMost(1, board -> max_x - 3); food -> y = randomAtMost(1, board -> max_y - 3);
	}

}

void addBonusFood(Board *board) {
	int new_x = randomAtMost(1, board -> max_x - 3); int new_y = randomAtMost(1, board -> max_y - 3);
	struct LinkedItem *bonus_food = malloc(sizeof(struct LinkedItem));
	bonus_food -> x = new_x; bonus_food -> y = new_y;

	while( inSamePlace(bonus_food, board -> snake) || (bonus_food -> x == board -> food -> x &&
	 bonus_food -> y == board -> food -> y)) {
		bonus_food -> x = randomAtMost(1, board -> max_x - 3); bonus_food -> y = randomAtMost(1, board -> max_y - 3);		
	}

	time_t start_t;
	time(&start_t);
	bonus_food -> active_time = start_t;

	bonus_food -> symbol = '$';
	board -> bonus_food = bonus_food;
}

bool oppositeDirection(struct LinkedItem *next_head, struct LinkedItem *snake) {
	if( (next_head -> direction == UP && snake -> direction == DOWN) || 
		(next_head -> direction == DOWN && snake -> direction == UP) ) {
		return true; 
	}
	if( (next_head -> direction == RIGHT && snake -> direction == LEFT) ||
		(next_head -> direction == LEFT && snake -> direction == RIGHT) ) {
		return true;
	}
	return false;
}

enum Status moveSnake(Board *board) {
	struct LinkedItem *head = board -> snake;
	struct LinkedItem *next_head = malloc(sizeof(struct LinkedItem));
	enum Directions d = board -> snake -> direction;

	switch(d) {		/* Find next cell */
		case UP:
			next_head -> x 			= (head -> x);
			next_head -> y 			= (head -> y) - 1;
			next_head -> symbol = '^';
			break;
		case DOWN:
			next_head -> x 			= (head -> x);
			next_head -> y 			= (head -> y) + 1;
			next_head -> symbol = 'v';
			break;
		case LEFT:
			next_head -> x 			= (head -> x) - 1;
			next_head -> y 			= (head -> y);
			next_head -> symbol = '<';
			break;
		case RIGHT:
			next_head -> x 			= (head -> x) + 1;
			next_head -> y 			= (head -> y);
			next_head -> symbol = '>';
			break;
		default:
			next_head -> x 			= (head -> x);
			next_head -> y 			= (head -> y);
			next_head -> symbol = head -> symbol;
			break;
	}

	/* Check if bonus food has expired */
	if( board -> bonus_food != NULL) {
		time_t end_t;
		time(&end_t);
		if( difftime(end_t, board -> bonus_food -> active_time) >= 5.0) {
			free(board -> bonus_food);
			board -> bonus_food = NULL;
		}
	}

	/* Check collision with food */
	if(next_head -> x == board -> food -> x && next_head -> y == board -> food -> y) {
		next_head -> next = head;
		next_head -> direction = head -> direction;
		head -> symbol = 'o'; 
		board -> snake = next_head;
		board -> score += 50;
		addFood(board);

		/* Check conditions for bonus food */
		board -> food_until_bonus ++;
		if(board -> food_until_bonus == 5) {
			addBonusFood(board);
			board -> food_until_bonus = 0;
		}

		return GAME_ON;
	}

	/* Check collision with bonus food */ 
	if( board -> bonus_food != NULL) {
		if(next_head -> x == board -> bonus_food -> x && next_head -> y == board -> bonus_food -> y) {
			board -> score += 150;
			free(board -> bonus_food);
			board -> bonus_food = NULL;
		}
	}

	/* Check if last snake cell is behind the head */
	if( oppositeDirection(head, head -> next) ) {
		free(next_head);
		/* Flip it so that the snake is still going in the right direction */
		switch(d) {   
			case UP:
				board -> snake -> direction = DOWN;
				return GAME_ON;
			case DOWN:
				board -> snake -> direction = UP;
				return GAME_ON;
			case LEFT:
				board -> snake -> direction = RIGHT;
				return GAME_ON;
			case RIGHT:
				board -> snake -> direction = LEFT;
				return GAME_ON;
			default:
			 	return GAME_ON;
		}
	}

	/* In borders and check if it's an open game or closed */
	if( next_head -> x <= 0 || next_head -> x >= board -> max_x - 1
		|| next_head -> y <= 0 || next_head -> y >= board -> max_y - 1) {
		switch(board -> option) {
			case OPEN:
				if(next_head -> x == board -> max_x - 1)
					next_head -> x = 1;

				if(next_head -> x == 0)
					next_head -> x = board -> max_x - 2;

				if(next_head -> y == board -> max_y - 1)
					next_head -> y = 1;
				if(next_head -> y == 0)
					next_head -> y = board -> max_y - 2;
				break;
			case CLOSED: 
			  free(next_head);
			  return GAME_OVER;
		}
	}
	
	/* Check if it's eating itself */
	if( inSamePlace(next_head, head -> next) ) {
		free(next_head);
		return GAME_OVER;
	}else {
		/* Swapping next_head with head and then the rest of the snake */
		struct LinkedItem hold;
		while(head) {
			hold.x = head -> x; hold.y = head -> y;
			head -> x 			= next_head -> x;
			head -> y 			= next_head -> y;
			head -> symbol 	= next_head -> symbol;
			head -> direction = d;		/* Tell the whole snake which direction it is travelling in */
			next_head -> x = hold.x; next_head -> y = hold.y; next_head -> symbol = 'o';
			head = head -> next;
		}
		free(next_head);
		return GAME_ON;
	}
}

Board* createBoard(int max_x, int max_y, enum Option option) {
	srand(time(NULL));		/* Seed the random number for later use in the program */

	Board *board = malloc(sizeof(Board));

	struct LinkedItem *food = malloc(sizeof(struct LinkedItem));
	*food = createCell(max_x/2, max_y/2, '#');

	board -> snake 						= createSnake();
	board -> food  						= food;
	board -> max_x						= max_x;
	board -> max_y 						= max_y;
	board -> score 						= 0;
	board -> option 					= option;
	board -> food_until_bonus = 0;
	board -> bonus_food				= NULL;

	/* Snake starts going down */
	board -> snake -> direction 	= HOLD;

	return board;
}

void destroyBoard(Board *board) {
	/* Free the food */
  free(board -> food);
  if(board -> bonus_food != NULL) {
    free(board -> bonus_food);
  }

  /* Free the snake */
  struct LinkedItem *head = board -> snake;
  struct LinkedItem *hold;
  while(head) {
    hold = head;
    head = head -> next;
    free(hold);
  }

  /* Finally free the board */
  free(board);
}

void writeScoreToFile(Board *board) {
	FILE *fp;

	switch(board -> option) {
		case OPEN:
			fp = fopen("high-scores-open.txt", "r");
			break;
		case CLOSED:
			fp = fopen("high-scores-closed.txt", "r");
			break;
	}

	int hold; char *ptr;

	char f_str[70], s_str[70], l_str[70];
	char *f_s, *s_s, *l_s;
	f_s = fgets(f_str, 70, fp); s_s = fgets(s_str, 70, fp); l_s = fgets(l_str, 70, fp); 
	fclose(fp);

	switch(board -> option) {
		case OPEN:
			fp = fopen("high-scores-open.txt", "w");
			break;
		case CLOSED:
			fp = fopen("high-scores-closed.txt", "w");
			break;
	}

	if(f_s == NULL) {
		fprintf(fp, "%ld\n", board -> score);
		fclose(fp);
		return;
	}else if(s_s == NULL) {

		/* highest to lowest*/
		hold = strtol(f_str, &ptr, 10);
		if( board -> score > hold ) {
			fprintf(fp, "%ld\n", board -> score);
			fprintf(fp, "%s", f_str);
		}else {
			fprintf(fp, "%s", f_str);
			fprintf(fp, "%ld\n", board -> score);
		}

		fclose(fp);
		return;
	}else if(l_s == NULL) {

		hold = strtol(f_str, &ptr, 10);
		if( board -> score > hold ) {
			fprintf(fp, "%ld\n", board -> score);
			fprintf(fp, "%s", f_str);
			fprintf(fp, "%s", s_str);
		}else if( board -> score > (hold = strtol(s_str, &ptr, 10)) ) {
			fprintf(fp, "%s", f_str);
			fprintf(fp, "%ld\n", board -> score);
			fprintf(fp, "%s", s_str);
		}else {
			fprintf(fp, "%s", f_str);
			fprintf(fp, "%s", s_str);
			fprintf(fp, "%ld\n", board -> score);
		}

		fclose(fp);
		return;
	}else {

		hold = strtol(f_str, &ptr, 10);
		if( board -> score > hold ) {
			fprintf(fp, "%ld\n", board -> score);
			fprintf(fp, "%s", f_str);
			fprintf(fp, "%s", s_str);
		}else if( board -> score > (hold = strtol(s_str, &ptr, 10)) ) {
			fprintf(fp, "%s", f_str);
			fprintf(fp, "%ld\n", board -> score);
			fprintf(fp, "%s", s_str);
		}else if ( board -> score > (hold = strtol(l_str, &ptr, 10)) ) {
			fprintf(fp, "%s", f_str);
			fprintf(fp, "%s", s_str);
			fprintf(fp, "%ld\n", board -> score);
		}else {
			fprintf(fp, "%s", f_str);
			fprintf(fp, "%s", s_str);
			fprintf(fp, "%s", l_str);
		}

		fclose(fp);
		return;
	}

}

