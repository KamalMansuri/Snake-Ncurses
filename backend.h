#include <stdbool.h>
#include <time.h>

enum Directions {UP, DOWN, LEFT, RIGHT, HOLD};
enum Option {OPEN, CLOSED};
enum Status {GAME_ON, GAME_OVER};

/* LinkedItem is used to repersent both snake and food */
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

struct LinkedItem createCell(int x, int y, char symbol);
struct LinkedItem *createSnake();
enum Status moveSnake(Board *board);
bool inSamePlace(struct LinkedItem *new_item, struct LinkedItem *snake);
int randomAtMost(int min ,int max);
Board* createBoard(int max_x, int max_y, enum Option option); 
void writeToScoreToFile(Board *board);
bool oppositeDirection(struct LinkedItem *next_head, struct LinkedItem *snake);
void addBonusFood(Board *board);
void destroyBoard(Board *board);
