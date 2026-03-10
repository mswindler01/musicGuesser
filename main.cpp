#include <iostream>
#include "levels.hpp"
#include "screen.h"


using namespace std;

int main(int argc, char* args[])
{  
    Level l;
    string guess;
    string type;
    string playAgain = "y";

    cout<<"Hello! Welcome to my music guessing game."<< endl;
    while (playAgain == "y" || playAgain == "Y") {
        cout<<"Would you like to guess by the lyrics or the rhythm?"<<endl;
        if (!(cin >> type)) {
            break;
        }

        if(type == "lyrics"){
            string lyricsAnswer;
            if (l.lyrics(lyricsAnswer) != 0) {
                return 1;
            }

            cout << "Enter song title: ";
            getline(cin >> ws, guess);

            if (guess == lyricsAnswer)
            {
                cout<<"You guessed correctly!"<<endl;
            } 
            else
            {
                cout << "You guessed wrong"<<endl;
            }
        }
        else if (type == "rhythm"){
            string rhythmAnswer;

            if (l.rhythm(rhythmAnswer) != 0) {
                return 1;
            }

            cout << "Enter song title: ";
            getline(cin >> ws, guess);

            if (guess == rhythmAnswer)
            {
                cout<<"You guessed correctly!"<<endl;
            }
            else
            {
                cout << "You guessed wrong"<<endl;
            }
        }
        else{
            cout<<"Invalid type"<<endl;
        }

        cout << "Play again? (y/n): ";
        if (!(cin >> playAgain)) {
            break;
        }
    }

    return 0; //screen(argc, args);

}
