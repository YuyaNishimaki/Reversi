/*
 *  リバーシプログラム
 *
 *  参考サイト
 *  http://kamiya.hatenadiary.jp/entry/2014/09/06/010504
 *  http://www.es-cube.net/es-cube/reversi/sample/index.html
 *  edax
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
//#include <unistd.h>
//#include <ctype.h>

#define TRUE    1
#define FALSE   0
#define BLACK   (int)'o'
#define WHITE   (int)'x'
#define EMPTY   (int)'-'

int board[10][10] = {0};
int myStone; // 自分の石の色
int curTurn = WHITE; // 現在のターン, 初めはWHITEから
int reverseCnt;

char buff[256]; // 相手の手を格納する配列

void init();
void displayBoard();
void play();
int isEmpty(); // EMPTYがあるときTRUE
int isPutCheck();
int isPutStone(int flag, int x, int y);
int isReverseStone(int flag, int x, int y);
int reverseStone(int flag, int x, int y, int dx, int dy);
void putStone();
void randPutStone();
void maxPutStone();
void changeTurn();
void displayResult(); // 結果を表示する関数

/* AI用の追加要素 */
void stepPutStone();				//もらったAIのマネ
void calcOpenness(int x, int y);	//開放度を求めてopennessに代入する
void step1();						//AIのゲーム進行度による3つの打ち筋の1(通常型)
void step2();						//AIのゲーム進行度による3つの打ち筋の2(序盤優勢型)
void step3();						//AIのゲーム進行度による3つの打ち筋の3(終盤優勢型)
int countMyCorner();				//手番の人の保持する角の数を返す
int openness;						//マスの開放度(周囲8方向の空きマスの合計,低いほどいいらしい)

// 盤面の重み配列
int weight[10][10] = {
    {0,0,0,0,0,0,0,0,0,0},
	{0,30, -12, 0, -1, -1, 0, -12, 30,0},
	{0,-12, -15, -3, -3, -3, -3, -15, -12,0},
	{0,0, -3, 0, -1, -1, 0, -3, 0,0},
	{0,-1, -3, -1, -1, -1, -1, -3, -1,0},
	{0,-1, -3, -1, -1, -1, -1, -3, -1,0},
	{0,0, -3, 0, -1, -1, 0, -3, 0,0},
	{0,-12, -15, -3, -3, -3, -3, -15, -12,0},
	{0,30, -12, 0, -1, -1, 0, -12, 30,0},
	{0,0,0,0,0,0,0,0,0,0}
};

int turns; // ターン数

/* メイン関数 */
int main(void) {
    init();
    play();
    displayResult();
    
    return 0;
}

/* 初期設定関数 */
void init() {
    /* 盤面の初期化 */
    int i , j;
    for (i = 1; i < 9; i++) {
        for (j = 1; j < 9; j++) {
            if ((i == 4 && j == 4) || (i == 5 && j == 5)) {
                board[i][j] = BLACK;
            } else if ((i == 4 && j == 5) || (i == 5 && j == 4)) {
                board[i][j] = WHITE;
            } else {
                board[i][j] = EMPTY;
            }
        }
    }
    
	/* セルの情報初期化 */
//	initCells(); //
	
	
    /* プレイヤーに先攻か後攻か選ばせる */
    while (myStone != (int)'o' && myStone != (int)'x') {
        printf("先攻(x)か後攻(o)か選んでください．");
        int c = getchar(); // 入力を一文字読み込む
        switch (c) {
            case 'o':
            case 'x':
                myStone = c;
                break;
            default:
                break;
        }
        while(getchar() != '\n');   /* 不要なバッファの削除 */
    }
    turns = 0; //ターン数初期化
    displayBoard();
}

/* 現在の盤面を表示させる関数 */
void displayBoard() {
    printf("  a b c d e f g h\n");
    int i, j;
    for (i = 1; i < 9; i++) {
        printf("%d", i);
        for (j = 1; j < 9; j++) {
            printf(" %c", board[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

/* リバーシを実行する関数 */
void play() {
    while(isEmpty()) {
        if (isPutCheck()) {
            if (curTurn == myStone) putStone();
            else                    stepPutStone();
            displayBoard();
        }
        changeTurn();
    }
}

/* まだ盤面に置けるか確認する関数 */
int isEmpty() {
    int i, j;
    for (i = 1; i < 9; i++) {
        for (j = 1; j < 9; j++) {
            if (board[i][j] == EMPTY)   return TRUE;
        }
    }
    return FALSE;
}

/* プレイヤーが置けるか確認する関数 */
int isPutCheck() {
    int i, j;
    for (i = 1; i < 9; i++) {
        for (j = 1; j < 9; j++){
            if (isPutStone(FALSE, i, j))    return TRUE;
        }
    }
    return FALSE;
}

/* 石が置けるかどうか確認する関数 */
int isPutStone(int flag, int x, int y) {
    /* 置く場所が盤面の範囲外でないかどうか */
    if (x < 1 || x > 8) return FALSE;
    if (y < 1 || y > 8) return FALSE;
    /* 置く場所に既に石がないかどうか */
    if (board[y][x] != EMPTY) return FALSE;
    /* 石をひっくり返せるかどうか */
    if (isReverseStone(flag, x, y)) return TRUE;
    
    return FALSE;
}

/* 石をひっくり返せるか確認する関数 */
int isReverseStone(int flag, int x, int y) {
    int flagCnt = 0;
    reverseCnt = 0;
    flagCnt += reverseStone(flag, x, y - 1, 0, -1);       /* 上方向 */
    flagCnt += reverseStone(flag, x + 1, y - 1, 1, -1);   /* 右上方向 */
    flagCnt += reverseStone(flag, x + 1, y, 1, 0);        /* 右方向 */
    flagCnt += reverseStone(flag, x + 1, y + 1, 1, 1);    /* 右下方向 */
    flagCnt += reverseStone(flag, x, y + 1, 0, 1);        /* 下方向 */
    flagCnt += reverseStone(flag, x - 1, y + 1, -1, 1);   /* 左下方向 */
    flagCnt += reverseStone(flag, x - 1, y, -1, 0);       /* 左方向 */
    flagCnt += reverseStone(flag, x - 1, y - 1, -1, -1);  /* 左上方向 */
    if (reverseCnt != 0)    return TRUE;
    else                    return FALSE;
}

/* 石をひっくり返す関数 */
int reverseStone(int flag, int x, int y, int dx, int dy) {
    /* 盤面の範囲外を指定した場合 */
    if (x < 1 || x > 8) return FALSE;
    if (y < 1 || y > 8) return FALSE;
    /* 石が置かれていない場合 */
    if (board[y][x] == EMPTY)   return FALSE;
    /* 同じ色の石があった場合 */
    if (board[y][x] == curTurn) return TRUE;
    else {
        /* 次の座標の石がひっくり返れば，ひっくり返す */
        if (reverseStone(flag, x + dx, y + dy, dx, dy)) {
            if(flag)    board[y][x] = curTurn;
            reverseCnt++;
            return TRUE;
        } else  return FALSE;
    }
}

/* プレイヤーに置く場所を入力させる関数 */
void putStone() {
    char str[32];
    char *tok;
    char *temp;
    int putX, putY;
    
    while (!isPutStone(TRUE, putX, putY)) {
        printf("石を置く場所を入力してください．");
        fgets(str, sizeof(str), stdin);
        tok = strtok(str, " ,");
        strcpy(temp, tok);
        putY = atoi(tok);
        tok = strtok(NULL, " ,");
        if (tok == NULL)    continue;
        strcat(buff, tok);
        char *p;  
        p = strchr(buff, '\n');  
        if(p != NULL) *p = '\0';
        strcat(buff, temp);
        putX = (int)tok[0] - (int)'a' + 1;
    }
    printf(buff);
    board[putY][putX] = curTurn;
}


/* ランダムで置く場所を決定するAI */
void randPutStone() {
    int putX, putY;
    printf("コンピュータの手．");
    while (!isPutStone(TRUE, putX, putY)) {
        // srand((unsigned) time(NULL));
        putY = (int)(rand() % 8 + 1);
        putX = (int)(rand() % 8 + 1);
    }
    printf("%d,%c\n", putY, putX + (int)'a' - 1);
    board[putY][putX] = curTurn;
}

/* 最大となる手を打つAI */
void maxPutStone() {
    int putX, putY;
    int tmpMax = 0;
    printf("コンピュータの手．");
    int i, j;
    for (i = 1; i < 9; i++) {
        for (j = 1; j < 9; j++) {
            if (isPutStone(FALSE, i, j)) {
                if (tmpMax < reverseCnt) {
                    tmpMax = reverseCnt;
                    putX = i;
                    putY = j;
                }
            }
        }
    }
    int put = isPutStone(TRUE, putX, putY);
    printf("%d,%c\n", putY, putX + (int)'a' - 1);
    board[putY][putX] = curTurn;
}

/* もらったAIのマネ */
void stepPutStone(){
	
	printf("コンピュータの手．%d", turns);
	
	int corner = countMyCorner(); //手番の人の保持する角の数を返す
	if(corner < 2)        step1();
	else if(turns <= 40)  step2();
	else                  step3();
	
}

/* 手番の人の保持する角の数を返す */
int countMyCorner(){
    int count = 0;
	if(board[1][1] == curTurn) {
	    count++;
	    weight[1][2] = weight[1][1];
    	weight[2][1] = weight[1][1];
    	weight[2][1] = weight[1][1];
	}
	
	if(board[1][8] == curTurn) {
	    count++;
	    weight[1][7] = weight[1][8];
    	weight[2][8] = weight[1][8];
    	weight[2][7] = weight[1][8];
	}
	
	if(board[8][1] == curTurn) {
	    count++;
	    weight[8][2] = weight[8][1];
    	weight[7][1] = weight[8][1];
    	weight[7][2] = weight[8][1];
	}
	
	if(board[8][8] == curTurn) {
	    count++;
	    weight[8][7] = weight[8][8];
    	weight[7][8] = weight[8][8];
    	weight[7][7] = weight[8][8];
	}
	
	return count;
}

/* 盤面の重みを重視し，取れる相手のコマが少ないものを選ぶ */
void step1(){
    printf("step1を実行");
	int putX, putY;
	int tmpMax = 0;
    int i, j;
    int cnt_flag = 0;
    for (i = 1; i < 9; i++) {
        for (j = 1; j < 9; j++) {
            if (isPutStone(FALSE, i, j)) {
                if (cnt_flag == 0) {
                    tmpMax = reverseCnt * weight[j][i];
                    putX = i;
                    putY = j;
                    cnt_flag = 1;
                } else if ((reverseCnt * weight[j][i] > tmpMax) && (cnt_flag == 1)) {
                    tmpMax = reverseCnt * weight[j][i];
                    putX = i;
                    putY = j;
                }
            }
        }
    }
    int put = isPutStone(TRUE, putX, putY);
	printf("%d,%c\n", putY, putX + (int)'a' - 1);
    board[putY][putX] = curTurn;
}

/* 開放度の低いものを重視し，取れる相手のコマが少ないものを選ぶ */
void step2(){
    printf("step2を実行");
	int putX, putY;
	int tmpMax = INT_MAX;
    int i, j;
    for (i = 1; i < 9; i++) {
        for (j = 1; j < 9; j++) {
            if (isPutStone(FALSE, i, j)) {
                calcOpenness(i, j);
                if (reverseCnt * openness < tmpMax) {
                    tmpMax = reverseCnt * openness;
                    putX = i;
                    putY = j;
                }
            }
        }
    }
    int put = isPutStone(TRUE, putX, putY);
	printf("%d,%c\n", putY, putX + (int)'a' - 1);
    board[putY][putX] = curTurn;
}

/* 取れる相手のコマが1番多いものを選ぶ．隅が空いていたら隅に置く */
void step3(){
    printf("step3を実行");
	int putX, putY;
	int tmpMax = 0;
	int cornerMax = 0;
    int i, j;
    int cnt_flag = 0;
    for (i = 1; i < 9; i++) {
        for (j = 1; j < 9; j++) {
            if (isPutStone(FALSE, i, j)) {
                if((i == 1 && j == 1) || (i == 1 && j == 8) || (i == 8 && j == 1) || (i == 8 && j == 1)) {
                    if (reverseCnt > cornerMax) {
                        tmpMax = reverseCnt;
                        cornerMax = reverseCnt;
                        putX = i;
                        putY = j;
                        cnt_flag = 1;
                    }
                }
                if ((reverseCnt > tmpMax) && (cnt_flag != 1)) {
                    tmpMax = reverseCnt;
                    putX = i;
                    putY = j;
                    cnt_flag = 2;
                }
            }
        }
    }
    int put = isPutStone(TRUE, putX, putY);
	printf("%d,%c\n", putY, putX + (int)'a' - 1);
    board[putY][putX] = curTurn;
}

/* 開放度を測る関数(置けない場合は-1を返す) */
void calcOpenness(int x, int y) {
    openness = 0;
	if(isPutStone(FALSE, x, y)){
		if (board[y - 1][x] == EMPTY)        openness++;
		if (board[y + 1][x] == EMPTY)        openness++;
		if (board[y - 1][x + 1] == EMPTY)    openness++;
		if (board[y + 1][x + 1] == EMPTY)    openness++;
		if (board[y - 1][x - 1] == EMPTY)    openness++;
		if (board[y + 1][x - 1] == EMPTY)    openness++;
		if (board[y][x + 1] == EMPTY)        openness++;
		if (board[y][x - 1] == EMPTY)        openness++;
	}else                                    openness = -1;
    
}

void initCells(){
}

/* ターンを変更する関数 */
void changeTurn() {
    if (curTurn == WHITE)   curTurn = BLACK;
    else                    curTurn = WHITE;
	turns++;
}

/* 結果を表示する関数 */
void displayResult() {
    int i, j;
    int bCnt = 0, wCnt = 0;
    for (i = 1; i < 9; i++) {
        for (j = 1; j < 9; j++) {
            if (board[i][j] == BLACK)   bCnt++;
            else                        wCnt++;
        }
    }
    printf("o:%d\nx:%d\nwinner:", bCnt, wCnt);
    if (bCnt > wCnt)    printf("o\n");
    else                printf("x\n");
}