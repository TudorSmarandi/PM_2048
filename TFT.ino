#include <SPI.h>
#include <TFT.h>
#include <IRremote.h>

#define CS   10
#define DC   9
#define RST  8

#define SIZE 4

#define UP 0xFF18E7
#define DOWN 0xFF4AB5
#define LEFT 0xFF10EF
#define RIGHT 0xFF5AA5
#define RESET 0xFF02FD
#define WIN 0xFF22DD
#define LOSE 0xFFC23D

const int RECV_PIN = 7;
const int RESET_PIN = 2;

IRrecv irrecv(RECV_PIN);
decode_results results;

TFT screen = TFT(CS, DC, RST);

int board[SIZE][SIZE] = { 0 };
int receivedInput = 1;
int over = 0;
int score = 0;

void(* resetFunc) (void) = 0;

void IRinput(void);

void drawBorders(void);
void drawGame(void);
void colorPicker(int i);
void displayScore1(void);
void displayScore2(void);
void gameEndMessage(void);

void resetGame(void);
void winGame(void);
void loseGame(void);

void moveDown(void);
void moveUp(void);
void moveRight(void);
void moveLeft(void);
void generateRandom(void);
int gameOver(void);
int hasSpaces(int line, char direction);

void setup() {
  screen.begin();
  screen.background(198, 198, 255);
  
  digitalWrite(RESET_PIN, HIGH);
  pinMode(RESET_PIN, OUTPUT);
  
  irrecv.enableIRIn();
  irrecv.blink13(true);
  
  randomSeed(analogRead(A0));
  generateRandom();
  displayScore1();
}

void loop() {
  if (over == 1)
    return;
  IRinput();
  if (receivedInput) {
    receivedInput = 0;
    drawGame();
    int res = gameOver();
    if (res > 0) {
      gameEndMessage(res);
      over = 1;
      delay(5000);
      resetGame();
      return;
    }
    displayScore2();
  }
}

void IRinput(void) {
  
  if (irrecv.decode(&results)){
        switch(results.value) {
          case UP: {
            moveLeft();
            receivedInput = 1;
            break;
          }
          case DOWN: {
            moveRight();
            receivedInput = 1;
            break;
          }
          case RIGHT: {
            moveDown();
            receivedInput = 1;
            break;
          }
          case LEFT: {
            moveUp();
            receivedInput = 1;
            break;
          }
          case RESET: {
            resetGame();
            receivedInput = 1;
            break;
          }
          case WIN: {
            winGame();
            receivedInput = 1;
            break;
          }
          case LOSE: {
            loseGame();
            receivedInput = 1;
            break;
          }
          default: {
            break;
          }
        }
        irrecv.resume();
  }
}

void drawBorders(void) {
  screen.stroke(0, 0, 0);
  
  screen.line(20, 10, 20, 90);
  screen.line(20, 90, 140, 90);
  screen.line(140, 90, 140, 10);
  screen.line(140, 10, 20, 10);

  screen.line(20, 30, 140, 30);
  screen.line(20, 50, 140, 50);
  screen.line(20, 70, 140, 70);

  screen.line(50, 10, 50, 90);
  screen.line(80, 10, 80, 90);
  screen.line(110, 10, 110, 90);
}

void drawGame(void) {
  int i, j;
  for (i = 0; i < SIZE; i++) {
    for (j = 0; j < SIZE; j++) {
      char cell[6];
      sprintf(cell, "%d", board[i][j]);
      colorPicker(board[i][j]);
      screen.rect(20 + 30 * i, 10 + 20 * j, 30, 20);
      if (board[i][j] == 0 || board[i][j] >= 2048)
        screen.stroke(255, 255, 255);
      else
        screen.stroke(0, 0, 0);
      screen.text(cell, 23 + 30 * i, 15 + 20 * j);
    }
  }
  drawBorders();
}

void colorPicker(int i) {
  switch (i) {
    case 0: {
      screen.fill(255, 255, 255);
      screen.stroke(255, 255, 255);
      break;
    }
    case 2: {
      screen.stroke(0, 255, 255);
      screen.fill(0, 255, 255);
      break;
    }
    case 4: {
      screen.stroke(0, 127, 255);
      screen.fill(0, 127, 255);
      break;
    }
    case 8: {
      screen.stroke(0, 0, 255);
      screen.fill(0, 0, 255);
      break;
    }
    case 16: {
      screen.stroke(0, 255, 0);
      screen.fill(0, 255, 0);
      break;
    }
    case 32: {
      screen.stroke(255, 255, 0);
      screen.fill(255, 255, 0);
      break;
    }
    case 64: {
      screen.stroke(255, 145, 0);
      screen.fill(255, 145, 0);
      break;
    }
    case 128: {
      screen.stroke(255, 0, 0);
      screen.fill(255, 0, 0);
      break;
    }
    case 256: {
      screen.stroke(250, 82, 127);
      screen.fill(250, 82, 127);
      break;
    }
    case 512: {
      screen.stroke(240, 0, 100);
      screen.fill(240, 0, 100);
      break;
    }
    case 1024: {
      screen.stroke(255, 0, 255);
      screen.fill(255, 0, 255);
      break;
    }
    case 2048: {
      screen.stroke(0, 0, 0);
      screen.fill(0, 0, 0);
      break;
    }
    default: {
      screen.stroke(0, 0, 0);
      screen.fill(0, 0, 0);
      break;
    }
  }
}

void displayScore1(void) {
  screen.fill(0, 0, 0);
  screen.rect(10, 100, 70, 25);
  screen.setTextSize(2);
  screen.stroke(255, 255, 255);
  screen.text("Score", 15, 105);
  screen.setTextSize(1);
}

void displayScore2(void) {
  char display_score[10];
  screen.fill(0, 0, 0);
  screen.rect(80, 100, 70, 25);
  screen.setTextSize(2);
  screen.stroke(255, 255, 255);
  screen.text("Score", 15, 105);
  sprintf(display_score, "%d", score);
  screen.text(display_score, 85, 105);
  screen.setTextSize(1);
}

void gameEndMessage(int result) {
  screen.fill(0, 0, 0);
  screen.rect(10, 100, 140, 25);
  screen.setTextSize(2);
  
  if (result == 2) {
    screen.stroke(0, 255, 255);
    screen.text("YOU WIN!", 35, 105);
  }
  else {
    screen.stroke(0, 0, 255);
    screen.text("YOU LOSE!", 30, 105);
  }
  screen.setTextSize(1);
}

void resetGame(void) {
  digitalWrite(RESET_PIN, LOW);
}

void winGame(void) {
  board[0][0] = 2048;
}

void loseGame(void) {
  board[0][0] = 1024;
  board[0][1] = 512;
  board[0][2] = 256;
  board[0][3] = 128;

  board[1][0] = 64;
  board[1][1] = 32;
  board[1][2] = 16;
  board[1][3] = 8;

  board[2][0] = 4;
  board[2][1] = 2;
  board[2][2] = 1024;
  board[2][3] = 512;

  board[3][0] = 256;
  board[3][1] = 128;
  board[3][2] = 64;
  board[3][3] = 32;
}

int gameOver(void) {
    int over = 1;
    
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++) {
          if (board[i][j] >= 2048)
            return 2;
          if (board[i][j] == 0)
            over = 0; 
        }
    if (over == 0)
      return over;
    for (int i = 0; i < SIZE - 1; i++)
        for (int j = 0; j < SIZE - 1; j++)
          if (board[i][j] == board[i + 1][j] || board[i][j] == board[i][j + 1])
            return 0;
    for (int i = 0; i < SIZE - 1; i++) {
      if (board[SIZE - 1][i] == board[SIZE - 1][i + 1])
        return 0;
      if (board[i][SIZE - 1] == board[i + 1][SIZE - 1])
        return 0;
    }
    return over;
}

int hasSpaces(int line, char direction) {
    if (direction == 'l') {
        for (int i = 0; i < SIZE - 1; i++) {
            if (board[line][i] == 0) {
                for (int j = i + 1; j < SIZE; j++) {
                    if (board[line][j] > 0)
                        return 1;
                }
            }
        }
    }

    if (direction == 'r') {
        for (int i = SIZE - 1; i > 0; i--) {
            if (board[line][i] == 0) {
                for (int j = i - 1; j >= 0; j--) {
                    if (board[line][j] > 0)
                        return 1;
                }
            }
        }
    }

    if (direction == 'u') {
        for (int i = 0; i < SIZE - 1; i++) {
            if (board[i][line] == 0) {
                for (int j = i + 1; j < SIZE; j++) {
                    if (board[j][line] > 0)
                        return 1;
                }
            }
        }
    }
    
    if (direction == 'd') {
        for (int i = SIZE - 1; i > 0; i--) {
            if (board[i][line] == 0) {
                for (int j = i - 1; j >= 0; j--) {
                    if (board[j][line] > 0)
                        return 1;
                }
            }
        }
    }

    return 0;
}

void generateRandom(void)
{
    while(1) {
        int x = random(4);
        int y = random(4);
        if (board[x][y] == 0) {
            board[x][y] = 2;
            break;
        }
    }
}

void moveLeft(void)
{
    int moved = 0;

    for (int i = 0; i < SIZE ; i++) {
        while (hasSpaces(i, 'l') == 1)
            for (int j = 1; j < SIZE; j++) {
                if (board[i][j - 1] == 0) {
                    board[i][j - 1] = board[i][j];
                    board[i][j] = 0;
                    moved = 1;
                }
            }
    }

    for (int i = 0; i < SIZE ; i++) {
        for (int j = 1; j < SIZE; j++) {
            if (board[i][j - 1] == board[i][j] && board[i][j] > 0) {
                board[i][j] += board[i][j - 1];
                score += board[i][j - 1];
                board[i][j - 1] = 0;
                j++;
                moved = 1;
            }
        }
    }

    for (int i = 0; i < SIZE ; i++) {
        while (hasSpaces(i, 'l') == 1)
            for (int j = 1; j < SIZE; j++) {
                if (board[i][j - 1] == 0) {
                    board[i][j - 1] = board[i][j];
                    board[i][j] = 0;
                    moved = 1;
                }
            }
    }

    if (moved == 1)
        generateRandom();
}

void moveRight(void)
{
    int moved = 0;

    for (int i = 0; i < SIZE ; i++) {
        while (hasSpaces(i, 'r') == 1)
            for (int j = SIZE - 2; j >= 0; j--) {
                if (board[i][j + 1] == 0) {
                    board[i][j + 1] = board[i][j];
                    board[i][j] = 0;
                    moved = 1;
                }
            }
    }

    for (int i = 0; i < SIZE ; i++) {
        for (int j = SIZE - 2; j >= 0; j--) {
            if (board[i][j + 1] == board[i][j] && board[i][j] > 0) {
                board[i][j] += board[i][j + 1];
                score += board[i][j + 1];
                board[i][j + 1] = 0;
                j--;
                moved = 1;
            }
        }
    }

    for (int i = 0; i < SIZE ; i++) {
        while (hasSpaces(i, 'r') == 1)
            for (int j = SIZE - 2; j >= 0; j--) {
                if (board[i][j + 1] == 0) {
                    board[i][j + 1] = board[i][j];
                    board[i][j] = 0;
                    moved = 1;
                }
            }
    }

    if (moved == 1)
        generateRandom();
}

void moveUp(void)
{
    int moved = 0;

    for (int i = 0; i < SIZE ; i++) {
        while (hasSpaces(i, 'u') == 1)
            for (int j = 1; j < SIZE; j++) {
                if (board[j - 1][i] == 0) {
                    board[j - 1][i] = board[j][i];
                    board[j][i] = 0;
                    moved = 1;
                }
            }
    }

    for (int i = 0; i < SIZE ; i++) {
        for (int j = 1; j < SIZE; j++) {
            if (board[j - 1][i] == board[j][i] && board[j][i] > 0) {
                board[j][i] += board[j - 1][i];
                score += board[j - 1][i];
                board[j - 1][i] = 0;
                j++;
                moved = 1;
            }
        }
    }

    for (int i = 0; i < SIZE ; i++) {
        while (hasSpaces(i, 'u') == 1)
            for (int j = 1; j < SIZE; j++) {
                if (board[j - 1][i] == 0) {
                    board[j - 1][i] = board[j][i];
                    board[j][i] = 0;
                    moved = 1;
                }
            }
    }

    if (moved == 1)
        generateRandom();
}

void moveDown(void)
{
    int moved = 0;

    for (int i = 0; i < SIZE ; i++) {
        while (hasSpaces(i, 'd') == 1)
            for (int j = SIZE - 2; j >= 0; j--) {
                if (board[j + 1][i] == 0) {
                    board[j + 1][i] = board[j][i];
                    board[j][i] = 0;
                    moved = 1;
                }
            }
    }

    for (int i = 0; i < SIZE ; i++) {
        for (int j = SIZE - 2; j >= 0; j--) {
            if (board[j + 1][i] == board[j][i] && board[j][i] > 0) {
                board[j][i] += board[j + 1][i];
                score += board[j + 1][i];
                board[j + 1][i] = 0;
                j--;
                moved = 1;
            }
        }
    }

    for (int i = 0; i < SIZE ; i++) {
        while (hasSpaces(i, 'd') == 1)
            for (int j = SIZE - 2; j >= 0; j--) {
                if (board[j + 1][i] == 0) {
                    board[j + 1][i] = board[j][i];
                    board[j][i] = 0;
                    moved = 1;
                }
            }
    }

    if (moved == 1)
        generateRandom();
}
