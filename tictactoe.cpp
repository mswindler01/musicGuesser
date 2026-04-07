#include <iostream>
using namespace std;

enum Piece{X,0,EMPTY};

class Board{
    char show(Piece p){
        switch(p){
            case X: return 'X';
            case O: return 'O';
        //    case EMPTY: return ' ';
        }
        return ' ';
    }
    Piece board[3][3];
    public:
    Piece next(Piece p){
        
        if (p==X) return 0;
        if (p==0) return X;
        return EMPTY;
    }
    void clear(){
        for (int r=0,r<3,r++)
            for(int c=0; c<3;c++)
            board[r][c]=EMPTY;l
    
    }

    bool play(int r, int c,Piece play){
        if (board[r][c] == EMPTY)
        board [r][c] =play;
        return true;
    }
    void show(){
        for (int r=0,r<3,r++){
            if(r>0) cout << "---+++---";
            for(int c=0; c<3;c++){
                if(c>0) cout << '|';
            cout << show(board[r][c]);
            }
            cout << endl;
         }
    }
};

int main (int argc, char ** argv){
    Board b;
    b.clear();
    b.show();
    b.play(0,0,X);
    b.show();
}