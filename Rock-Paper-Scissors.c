#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

int game (char you, char computer){
    

    //Same selection
    if (you == computer)
    return -1;
    
    //Player ROCK & Computer Paper
    if (you == 's' && computer == 'p')
    return 0;

    //Player Paper & Computer ROCK
    else if (you == 'p' && computer == 's')
    return 1;

    //Player Scissors & Computer ROCK
    if(you == 'x' && computer == 's')
    return 0;

    //Player ROCK & Computer Scissors
    else if(you == 's' && computer == 'x')
    return 1;

    //Player Paper & Computer Scissors
    if(you == 'p' && computer == 'x')
    return 0;

    //Player Scissors & Computer Paper
    else if(you == 'x' && computer == 'p')
    return 1;
}

int main () {

    int n;

    char you, computer, result, again;

    do 
    {
    srand(time(NULL));

    n = rand() % 100;

    if (n < 33)
    computer = 's';

    else if (n > 33 && n < 66)
    computer = 'p';

    else
    computer = 'x';

    printf("\n\n\n\n\t\t\t\tEnter s for ROCK, p for PAPER and x for SCISSOR\n\t\t\t\t\t\t\t");

    scanf(" %c", &you);
    
    result = game(you, computer);
    
    
    if (result == -1) {
        printf("\n\n\t\t\t\tGame Draw!\n");
    }
    else if (result == 1) {
        printf("\n\n\t\t\t\tWow! You have won the game!\n");
    }
    else { 
        printf("\n\n\t\t\t\tOh! You have lost the game!\n");
    }
        printf("\t\t\t\tYOu choose : %c and Computer choose : %c\n",you, computer);
 
    
    Sleep(3000);

    printf("\t\t\t\tWould you like to play again? (y/n):");

    scanf(" %c", &again);

    } while (again == 'y');
    return 0;
}