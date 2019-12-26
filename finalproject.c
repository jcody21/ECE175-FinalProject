#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <conio.h> // included for the _getch() function

// Authors: John Cody & Sarah Huggins-Hubbard
// Date: 04/11/2018
// Program Description: Plays a game of Uno with the users. 

typedef struct card_s {
	char suit[7];
	int value;
	char action[15];
	struct card_s *pt;
} card;

typedef struct player_s {  // a struct to help identify the different player's hands
	card* hand;
	int score;
} player;

card *deck_from_file(FILE *input); // takes a FILE pointer, loops through the file to create all of the cards, and returns the head pointer
card *create_deck(void); // creates a deck in order based on the one shown in the project description
void print_hand(card *handptr);  // prints an entire hand that is passed to it
void print_card(card *card);  // prints just one card
void shuffle_deck(card *head);  // shuffles a deck or hand of any size
void swap_cards(card *head, int i, int j);  // the swap function that carries out the exchange of values for the shuffle function
void deal_cards(player *arr, card **head, int players, int cards); // takes the array of players, the deck, the num players, and amount of cards to be delt and deals that many cards to each player
void discardf(card **discard, card **hand, int num);  // takes the discard stack, the hand to pulled from, and which card in the list to discard, and then discards that one card
void print_discard(card* discard); // prints the 5 topmost cards of the discard stack 
void draw_card(player *arr, card **head, card* discard, int player, int cards);  // draws however many cards requested from the deck and adds them to the top or whichever hand indicated
void play(card **stack, card **discard, player *players, int j); // runs a players turn until they discard 
void redistribution(card *head, card *discard); // if the end of the deck is reached then the discard pile is pulled away and used to replensish the deck
void setup(card **deck); // abstracts much of the setup work such as prompting the user for input and calling the create_deck function
void run_game(player *players, card **deck, card **discard, char *win, char *direction, int numPlayers); // abstracts the running of the game - calls the play function and players in turn and executes action card powers
void allocate_points(player *players, int numPlayers, int winner); // allocates points to the winner of the round according to the UNO rules

int value; // Global variables that the discard function changes to be able to check if something is able to be discarded
char color[10];

int main(void) {
	int numPlayers, i = 0;
	char win = 'n', direction = 'f', userChar = 'y';
	card *discard = NULL, *deck = NULL, *temp = NULL;
	player *players = NULL;  // setting up variables and pointers

	printf("How many players will there be? (2-10): "); // prompting the user for input
	scanf(" %d", &numPlayers); // scanning for that input

	while ((numPlayers > 10) || (numPlayers < 2)) { // waiting until the user provides useful input
		printf("\nNumber entered is outside of the range provided. Please try again. \n");
		printf("How many players will there be? (2-10): ");
		scanf(" %d", &numPlayers);
	}

	players = (player *)malloc(numPlayers * sizeof(player)); // allocating memory for the number of players and saving it as a pointer which will act as a dynamically allocated array

	for (i = 0; i < numPlayers; i++) { // looping through the players just created and setting the hand pointer to NULL and the score value to 0
		(players + i)->hand = NULL;
		(players + i)->score = 0;
	}
	
	while (userChar == 'y') { // allowing the whole program to loop so if a player after winning wants to play again they can do so
		
		win = 'n';

		srand((int)time(NULL)); // changing the rand() seed so there is no replication
		
		setup(&deck); // using the pointers and setting up the linked list

		printf("Everything is ready. Press any key to deal cards. ");

		_getch(); // this just waits for the user to enter anything to begin the game

		deal_cards(players, &deck, numPlayers, 7); // dealing 7 cards to each of the players

		discardf(&discard, &deck, 0); // discarding the first card of the deck to begin the discard pile

		run_game(players, &deck, &discard, &win, &direction, numPlayers); // runs the game until there is a winner

		temp = discard; // setting the temp pointer to the beginning of the discard stack
		for (i = 0; i < numPlayers; i++) { // looping through all of the players to put all of their cards in the discard stack
			while (temp->pt != NULL) { // looping through the discard list until the last card
				temp = temp->pt;
			}
			temp->pt = (players + i)->hand; // setting the pointer of the last card to the players hand
			(players + i)->hand = NULL; // setting the players hand to NULL
		}

		free(deck); // frees up memory allocated for the different variables
		free(discard); 

		deck = NULL;
		discard = NULL;

		printf("\nThe current scores are: \n"); // Printing out the scores for the players
		for (i = 0; i < numPlayers; i++) {
			printf("Player %d's score is %d.\n", i + 1, (players + i)->score);
		}

		for (i = 0; i < numPlayers; i++) { //looping through the players 
			if ((players + i)->score > 500) { // tests to see if a player has won the game
				userChar = 'w'; // preparing for the lower statement to exit the big loop
				printf("\nPLAYER #%d HAS A SCORE OF %d AND HAS WON ALL OF UNO!\n", i + 1, (players + i)->score); // congratulating the winner
				break;
			}
		}
		
		if (userChar == 'w') { // exiting to prevent the lower statements from printing
			break;
		}

		printf("\nWould you like to continue to the next round? (y/n): "); // prompting the user if they want keep going
		scanf(" %c", &userChar);

		while ((userChar != 'n') && (userChar != 'y')) { // input protection
			printf("\nThat was not one of the options, please try again.\n");
			printf("\nWould you like to continue to the next round? (y/n): ");
			scanf(" %c", &userChar);
		} // if 'y' the program will start back at the setup
	}
	
	printf("\nBye bye\n\n"); // if 'n' the program says goodbye to the user

	return 0;  // fin
}

void run_game(player *players, card **deck, card **discard, char *win, char *direction, int numPlayers) /*This function forms the backbone of the program and does the looping through players that makes this a game*/ { 
	int i = 0, handSize = 0; // creating some variables
	char userChar;
	card *temp = NULL;

	while (*win != 'y') {
		while (*direction == 'f') { // loops this way while in the forward direction / a reverse card has not been played
			play(&(*deck), &(*discard), players, i); // calling the play function to run player i's turn
			handSize = 0;
			temp = (players + i)->hand; // resetting the temp pointer
			while (temp != NULL) { // looping through to see how many cards the player has
				temp = temp->pt;
				handSize++;
			}
			if (handSize == 1) { // if they have UNO or they have successfully discarded all of their cards, the will be notified
				printf("Player%d has UNO!\n", i + 1);
			}
			if (handSize == 0) {
				printf("Player%d has WON this round!\n", i + 1);
				allocate_points(players, numPlayers, i);
				*win = 'y'; // changing the win check variable so that the big loop breaks and the function exits
				break; // breaking to exit this loop to go to exit the function
			}
			if (value == 10) { // these next couple ifs check to see if an action card was played and implement the rules accordingly
				if (i + 1 == numPlayers) { // this one is for Skip, if this is the 9th of 9 players
					i = 0; // now it goes back to the beginning, and so that when incremented at the end, player 0 will be skipped
				}
				else { // if it is not at the end, then it just increments by 1, and when it increments again at the end of the loop, it will effectively skip
					i++;
				}
			}
			if (value == 11) { // this is for reverse
				*direction = 'r'; // it changes the direction
				if (numPlayers == 2) { // if the number of players is two it acts like a skip card and remains at the same user
					break;
				}
				else if (i == 0) {
					i = numPlayers - 1; // it decrements instead of incrementing
					break;
				}
				else {
					i--;
					break;
				}
			}
			if (value == 12) { // this is for draw two cards
				if (i == numPlayers - 1) { // if last card, the two are added to the first player
					draw_card(players, &(*deck), *discard, 0, 2);
				}
				else { // else, the two are added to the next player
					draw_card(players, &(*deck), *discard, i + 1, 2);
				}
			}
			if (value == 13) { // this is for wild/wild draw four cards

				if ((*discard)->action[4] == ' ') { // if the action string of the card has a space instead of a NULL characer, it is a draw four card
					if (i == numPlayers - 1) { // following the same rules as the draw two cards
						draw_card(players, &(*deck), *discard, 0, 4);
					}
					else {
						draw_card(players, &(*deck), *discard, i + 1, 4);
					}
				}

				printf("What would you like the new color to be? (R/G/B/Y): "); // prompting the user for input
				scanf(" %c", &userChar); // scanning for input 

				while ((userChar != 'R') && (userChar != 'G') && (userChar != 'B') && (userChar != 'Y')) { // waiting for the user to provide useable input
					printf("Letter entered was not one of the options provided. Please try again.\n");
					printf("What would you like the new color to be? (R/G/B/Y): ");
					scanf(" %c", &userChar);
				}

				if (userChar == 'R') { // assinging the global variable to the new color according to the input provided
					strcpy(color, "Red\0");
				}
				if (userChar == 'G') {
					strcpy(color, "Green\0");
				}
				if (userChar == 'B') {
					strcpy(color, "Blue\0");
				}
				if (userChar == 'Y') {
					strcpy(color, "Yellow\0");
				}
			}
			if (i + 1 >= numPlayers) { // incrementing to the next player appropriately
				i = 0;
			}
			else {
				i++;
			}

		}

		while (*direction == 'r') { // while going in the reverse direction
			play(&(*deck), &(*discard), players, i); // playing player i's turn
			handSize = 0; 
			temp = (players + i)->hand; // resetting the temp variable
			while (temp != NULL) { // looping to find the hand size of the player
				temp = temp->pt;
				handSize++;
			}
			if (handSize == 1) { // alerting the user if they have UNO or if they have won
				printf("Player%d has UNO!\n", i + 1);
			}
			if (handSize == 0) {
				printf("Player%d has WON this round!\n", i + 1);
				allocate_points(players, numPlayers, i);
				*win = 'y'; // changing the win variable so that the big loop breaks and the function exits to the main
				break; // breaking so that the reverse loop exits
			}
			if (value == 10) { // skip
				if (i == 0) { // providing end protection when incrementing
					i = numPlayers - 1;
				}
				else {
					i--;
				}
			}
			if (value == 11) { // reverse
				*direction = 'f'; // changes the direction to forward
				if (numPlayers == 2) {
					break;
				}
				if (i == numPlayers - 1) { // increments in the forward direction
					i = 0;
					break;
				}
				else {
					i++;
					break;
				}
			}
			if (value == 12) { // draw two
				if (numPlayers == 2) { // if the number of players is two it acts like a skip card and remains at the same user

				}
				else if (i == 0) { // applies the draw two to the appropriate player
					draw_card(players, &(*deck), *discard, numPlayers - 1, 2);
				}
				else {
					draw_card(players, &(*deck), *discard, i - 1, 2);
				}
			}
			if (value == 13) { // wild/wild draw four
				if ((*discard)->action[4] == ' ') { // checking to see if wild or wild draw four
					if (i == 0) { // applying the draw four to the appropriate player
						draw_card(players, &(*deck), *discard, numPlayers - 1, 4);
					}
					else {
						draw_card(players, &(*deck), *discard, i - 1, 4);
					}
				}

				printf("What would you like the new color to be? (R/G/B/Y): "); // prompting user for input
				scanf(" %c", &userChar); // scanning for input

				while ((userChar != 'R') && (userChar != 'G') && (userChar != 'B') && (userChar != 'Y')) { // waiting for the user to provide useable input
					printf("Letter entered was not one of the options provided. Please try again.\n");
					printf("What would you like the new color to be? (R/G/B/Y): ");
					scanf(" %c", &userChar);
				}

				if (userChar == 'R') { // assigning the global variable color based on the input provided by the user
					strcpy(color, "Red\0");
				}
				if (userChar == 'G') {
					strcpy(color, "Green\0");
				}
				if (userChar == 'B') {
					strcpy(color, "Blue\0");
				}
				if (userChar == 'Y') {
					strcpy(color, "Yellow\0");
				}
			}
			if (i <= 0) { // incrementing so as to go in reverse
				i = numPlayers - 1;
			}
			else {
				i--;
			}
		}
	}
}

void allocate_points(player *players, int numPlayers, int winner) {
	int i = 0;
	card *temp = NULL;

	for (i = 0; i < numPlayers; i++) {
		if (i != winner) {
			temp = (players + i)->hand;
			while (temp != NULL) {
				if (temp->value < 10) {
					(players + winner)->score += temp->value;
				}
				else if (temp->value < 13) {
					(players + winner)->score += 20;
				}
				else {
					(players + winner)->score += 50;
				}
				temp = temp->pt;
			}
		}
	}
}

void setup(card **deck) /*This function abstracts and carries out most of the setup functions*/ {
	int userNum = 0, i = 0; // creating some variables
	
	printf("Let's Play a Round of UNO!\n"); // welcoming the user to the game
	printf("Press 1 to shuffle the UNO deck or 2 to load a deck from a file: "); // prompting user for input
	scanf(" %d", &userNum); // scanning for the input

	while ((userNum != 1) && (userNum != 2)) { // waiting until the user gives useful input
		printf("\nThat was not one of the options, please try again.\n");
		printf("\nPress 1 to shuffle the ONE-O deck or 2 to load a deck from a file: ");
		scanf(" %d", &userNum);
	}

	if (userNum == 1) { // if the user elects to just create a deck and shuffle it
		*deck = create_deck(); // using the create deck function and a pointer to save the location of the list
		shuffle_deck(*deck); // shuffling the values with the shuffle_deck function
		printf("The deck is shuffled.\n"); // alerting the user as to what has happened
	}

	if (userNum == 2) { // if the user elects to load a deck from a file
		FILE* input = fopen("cards.txt", "r"); // creating a FILE pointer and opening a file to read
		*deck = deck_from_file(input); // using the deck_from_file function to create a list and return the pointer to that list
		fclose(input); // closing the file
		printf("The deck has been loaded from the file. \n"); // alerting the user as to what has happened
	}
}

void play(card **stack, card **discard, player *players, int j) /*This function plays one player's turn*/ {
	int userNum = -2, handSize = 0, i = 0; // creating some variables
	card *temp = NULL;
	char check = 'b';

	printf("\n"); // formatting
	print_discard(*discard); // printing out the discard pile for the user to read and know what card to play

	while (check == 'b') { // loops until the player discards - 'b' = bad, 'g' = good
		temp = (players + j)->hand; // resetting the temp pointer
		handSize = 0;
		while (temp != NULL) { // looping to find how many card the player now has as they could have just drawn a card
			temp = temp->pt;
			handSize++;
		}

		printf("\n\nPlayer%d's hand: ", j + 1); // formatting for the player's own hand
		print_hand((players + j)->hand); // printing out the cards in the player's hand

		printf("\n\nPress 1-%d to play any card from your hand, or press zero to draw a card from the deck: ", handSize); // prompting the user for input
		scanf(" %d", &userNum); // scanning for the input from the user

		while ((userNum < 0) && (userNum > handSize)) { // if the input is outside of the appropriate range, the user is forced to try again until they get it right
			printf("\nNumber entered is outside of the range provided. Please try again. \n");
			printf("Press 1-%d to play any card from your hand, or press zero to draw a card from the deck: ", handSize);
			scanf(" %d", &userNum);
		}

		if (userNum == 0) { // if 0, then they wanted to draw a card, so 1 card is drawn from the deck and added to their hand
			draw_card(players, &(*stack), discard, j, 1);
		}
		else { // otherwise, they wanted to try and discard one of their cards
			temp = (players + j)->hand; // resetting the temp pointer

			for (i = 0; i < (userNum - 1); i++) { // looping to get to the card indicated by the user
				temp = temp->pt;
			}

			if (check_discard(temp) == 1) { // checking to ensure that the card attempting to be discarded can actually be discarded according to the rules
				discardf(&(*discard), &((players + j)->hand), userNum - 1); // if yes, the card is discarded
				check = 'g'; // and the player's turn ends, so the program can stop looping
			}
			else { // if it can't be placed down, then the program outputs a message and they are promted for new input as the loop starts over
				printf("The "); 
				print_card(temp); // telling which card cannot be placed upon
				printf(" cannot be placed on top of ");
				print_card(*discard); // this other card
				printf(".\n");
				check = 'b'; // maintaing that the program needs to loop again
			}
		}
	}
}

int check_discard(card *discard) /*This function checks a card to ensure that it follows the game's rules and can be added to the discard pile*/ {
	if ((discard->value == value) || (discard->suit[0] == color[0]) || (discard->value == 13)) { // if the values match, the suits are the same, or it is a Wild/Wild Draw Four card
		return 1; // it is good and can be discarded
	}
	else { // otherwise, it cannot be discarded
		return 0;
	}
}

void draw_card(player *arr, card **head, card* discard, int player, int cards) /*This function draws however many cards from the deck for whichever player*/ {
	int j = 0; // creating a loop variable
	card *temp = NULL;

	for (j = 0; j < cards; j++) { // looping through however many cards need to be drawn
		temp = *head; // resetting the temp pointer
		*head = temp->pt; // setting the head to the second card, effectively removing the first card from the list

		if ((*head)->pt == NULL) { // checks to make sure that there is a card after the current one
			redistribution(*head, discard); // if there is not, the redistribution function "refills" the deck from the discard pile
		}

		temp->pt = (arr + player)->hand; // sets temp card to point to the hand 
		(arr + player)->hand = temp; // hand now points to temp, effectively adding the card to the beginning of the list
	}
}

void redistribution(card *head, card *discard) /*This function will redistribute the cards from the discard stack to the deck if the deck runs out of cards*/ {
	head->pt = discard->pt; // having the last card in the deck point to the second card of the discard pile
	discard->pt = NULL; // setting it so that there is now only one card in the discard pile
	shuffle_deck(head); // shuffling the new deck
}

void print_discard(card* discard) /*This function prints the top 5 cards of the discard stack */ {
	int counter = 0;
	card *temp = discard; 

	while (temp != NULL) { // looping to find out how many cards there are in the discard stack
		temp = temp->pt;
		counter++;
	}

	printf("\nDiscard pile: "); // first statement
	temp = discard; // resetting the temp pointer

	if (counter > 5) { // if greater then 5, the list has to be manually ouput
		counter = 1;
		print_card(temp); // printing the first card
		temp = temp->pt;
		while (counter < 5) { // looping through the 4 other cards
			printf(", "); // added to make the list look good
			print_card(temp); // printing whichever card the list is on
			temp = temp->pt; // incrementing the card in the list
			counter++; // incrementing the counter
		}
	}
	else { // otherwise, we can just use the already created print_hand function
		print_hand(discard);
	}
}

void discardf(card **discard, card **hand, int num) /*This function discards whichever card from the user's hand to the top of the discard pile*/ {
	int i = 0; // creating a loop variable
	card *temp = *hand, *previous = temp; 
	
	if (num > 0) { // pulling the card from the hand
		for (i = 0; i < num; i++) { // looping to get to the card to be pulled
			previous = temp;
			temp = temp->pt;
		}
		previous->pt = temp->pt; // setting the previous card to the card after the temp, so now the list skips the temp card
	}
	else { 
		*hand = temp->pt; // if temp is the first card, looping isn't necessary, so the hand pointer now points to the second card to skip temp
	}

	if (*discard == NULL) { // placing the card in the discard pile
		*discard = temp; // if the discard pile hasn't yet been started
		temp->pt = NULL;
	}
	else { // otherwise
		temp->pt = *discard; // temp now points to the discard stack
		*discard = temp; // and the discard pointer points to temp, so temp is now on top and points to the rest of the discard stack
	}

	value = temp->value; // altering the global variables to the characteristics of the card that was just discarded
	strcpy(color, temp->suit); // so that the check_discard function can compare and make sure the user follows the rules of the game
}

void deal_cards(player *arr, card **head, int players, int cards) /*This function deals however many cards to however many players*/ {
	int i = 0, j = 0; // creating some loop variables
	card *temp = NULL; 


	for (j = 0; j < cards; j++) { // looping through however many cards need to be handed out to each player
		for (i = 0; i < players; i++) { // looping through the players that need to get a card

			temp = *head; // taking the head pointer of the deck
			*head = temp->pt; // and incrementing it so that it now points to the second card

			if ((arr + i)->hand == NULL) { // if the hand doesn't point to anything yet
				(arr + i)->hand = temp; // setting the hand to the previous top card of the deck
				temp->pt = NULL; // setting the pointer in the card to NULL
			}
			else { // if the hand already has cards
				temp->pt = (arr + i)->hand; // the temp card now points to the top of the hand
				(arr + i)->hand = temp; // the hand now points to the temp card - essentially, the temp card has become the new top card
			}
		}
	}
}

card *deck_from_file(FILE *input) /*This function takes a FILE pointer and loops through a file to create a deck in that order*/ {
	char str[20]; // creating a string to hold the input from the file
	card *head = NULL, *tail = NULL, *temp = NULL; // pointers to do the linking of the list

	while (feof(input) != 1) { // big loop to go through the entire file
		temp = (card *)malloc(sizeof(card)); // allocating memory for a card
		if (head == NULL) { //this if/else group does the linking of the two cards, check create_deck for details on how this works
			head = temp;
			tail = temp;
			tail->pt = NULL;
		}
		else {
			tail->pt = temp;
			tail = temp;
			tail->pt = NULL;
		}
		fgets(str, 20, input); // getting input from the file

		if ((str[0] == '0') || (str[0] == '1') || (str[0] == '2') || (str[0] == '3') || (str[0] == '4') || (str[0] == '5') || (str[0] == '6') || (str[0] == '7') || (str[0] == '8') || (str[0] == '9')) {
			temp->value = (int)(str[0] - '0'); // The above checks to see if the first character is between 0-9 - this line sets the card value to that value as an int by subtracting '0' to get the proper number
			if (str[2] == 'R') { // these check to see what the third letter is to set the appropriate color
				strcpy(temp->suit, "Red\0");}
			if (str[2] == 'G') {
				strcpy(temp->suit, "Green\0");}
			if (str[2] == 'B') {
				strcpy(temp->suit, "Blue\0"); }
			if (str[2] == 'Y') {
				strcpy(temp->suit, "Yellow\0"); }
			strcpy(temp->action, "\0"); // sets the action to nothing for all number cards
		}
		else if (str[0] == 'S') { // this group does all the same things as the previous, but with slight changes for Skip cards
			temp->value = 10;
			if (str[5] == 'R') {
				strcpy(temp->suit, "Red\0");
			}
			if (str[5] == 'G') {
				strcpy(temp->suit, "Green\0");
			}
			if (str[5] == 'B') {
				strcpy(temp->suit, "Blue\0");
			}
			if (str[5] == 'Y') {
				strcpy(temp->suit, "Yellow\0");
			}
			strcpy(temp->action, "Skip\0");
		}
		else if (str[0] == 'R') { // all the same except for Reverse cards
			temp->value = 11;
			if (str[8] == 'R') {
				strcpy(temp->suit, "Red\0");
			}
			if (str[8] == 'G') {
				strcpy(temp->suit, "Green\0");
			}
			if (str[8] == 'B') {
				strcpy(temp->suit, "Blue\0");
			}
			if (str[8] == 'Y') {
				strcpy(temp->suit, "Yellow\0");
			}
			strcpy(temp->action, "Reverse\0");
		}
		else if (str[0] == 'D') { // same for Draw Two cards
			temp->value = 12;
			if (str[9] == 'R') {
				strcpy(temp->suit, "Red\0");
			}
			if (str[9] == 'G') {
				strcpy(temp->suit, "Green\0");
			}
			if (str[9] == 'B') {
				strcpy(temp->suit, "Blue\0");
			}
			if (str[9] == 'Y') {
				strcpy(temp->suit, "Yellow\0");
			}
			strcpy(temp->action, "Draw Two\0");
		}
		else if (str[0] == 'W') { // slightly different for Wild/Wild Draw Four cards
			temp->value = 13; // value is still set
			strcpy(temp->suit, "\0"); // suit doesn't matter to these cards, so it is blanked
			if (str[4] == '_') { // checking if there is an underscore to indicate that this is a Wild Draw Four instead of a Wild
				strcpy(temp->action, "Wild Draw Four\0");
			}
			else { // if no underscore, it must be just a Wild
				strcpy(temp->action, "Wild\0");
			}
		}
	}
	return head; // returing the head pointer to the entire linked list
}

void print_hand(card *handptr) /*This function prints a hand or linked list of any size*/ {
	card *hand = handptr; // creating a pointer to move through the list with

	if (hand->value < 10) { // printing the first card in the hand based on the rules in print_card
		printf("%d-%s", hand->value, hand->suit);
	}
	else if (hand->value < 13) {
		printf("%s-%s", hand->action, hand->suit);
	}
	else {
		printf("%s", hand->action);
	}
	hand = hand->pt; // incrementing the pointer to be able to print the next card 

	while (hand != NULL) { // looping until the entire hand is printed
		if (hand->value < 10) {
			printf(", %d-%s", hand->value, hand->suit); // only difference is the addition of the ", "
		}
		else if (hand->value < 13) {
			printf(", %s-%s", hand->action, hand->suit);
		}
		else {
			printf(", %s", hand->action);
		}
		hand = hand->pt; // incrementing the pointer to be able to print the next card
	}
}

void print_card(card *card) /*This function prints a single card*/ {
	if (card->value < 10) { // if it isn't an action card than you just print the number and color
		printf("%d-%s", card->value, card->suit);
	}
	else if (card->value < 13) { // if it is an action card that isn't wild or wild draw four then print the action and the color
		printf("%s-%s", card->action, card->suit);
	}
	else { // finally, it must be a wild or wild draw four, so the color isn't important, so the color isn't printed
		printf("%s", card->action);
	}
}

void shuffle_deck(card *head) /*This function shuffles the values of a linked list of any size*/ {
	int counter = 0, numSwaps = 1000, i = 0; // creating some variables
	card *temp = head;  

	while (temp->pt != NULL) { // looping to find how many cards are in a certain list
		temp = temp->pt;
		counter++;
	}

	counter++; // one final incrementation to get it to the appropriate size

	for (i = 0; i < numSwaps; i++) { // looping to complete a predetemined nunber of swaps
		swap_cards(head, rand()%counter, rand()%counter); // calling the swap function each time with a different random numbers
	}
}

void swap_cards(card *head, int i, int j) /*This function is used by shuffle_deck and swaps all the values held within two cards*/ {
	int x = 0; // initializing a loop variable
	
	char suit[7]; // initializing all the variables that will hold the data while replacing the other
	int value;
	char action[15] = "\0";
	
	card* card1 = head;  // creating pointers to hold the location of the cards to be switched
	card* card2 = head;
	
	for (x = 0; x < i; x++) { // looping to get to the specific cards to be switched
		card1 = card1->pt;
	}
	for (x = 0; x < j; x++) {
		card2 = card2->pt;
	}
	
	strcpy(suit, card1->suit); // holding the data before card1 is overwritten
	value = card1->value;
	strcpy(action, card1->action);

	strcpy(card1->suit, card2->suit); // overwiting card1 with card2's info
	card1->value = card2->value;
	strcpy(card1->action, card2->action);

	strcpy(card2->suit, suit); // writing card1's saved data into card2
	card2->value = value;
	strcpy(card2->action, action);
}

card *create_deck(void) /*This function creates a deck based on the inage provided in the project description*/{
	int i = 0, j = 0; // creating loop variables
	card *head = NULL, *tail = NULL, *temp = NULL; // creating variables to stitch together the list

	for (j = 0; j < 4; j++) { // loops through the first 4 rows to create the colors with 0's and just wild cards
		for (i = 0; i < 14; i++) { // creates a row by creating card by card and appropriately linking to the next one
			temp = (card *)malloc(sizeof(card)); // allocating memory for the each card in turn
			if (head == NULL) { // if it is the first, this executes
				head = temp; // starting the list
				tail = temp; // saving the current position of the tail
				tail->pt = NULL; // setting the pt to NULL until the next card is added
			}
			else { // otherwise, for every card after the first, it gets linked after the previous card
				tail->pt = temp; // setting the previous tail to now point to the current card
				tail = temp; // setting the tail now to the current card
				tail->pt = NULL; // setting the current tail pt to NULL until the next card is added, or it remains NULL
			}
			temp->value = i; // setting the number value of the current card so it actually means something - 0-9 are regular, 10-13 are action cards
			if (j == 0) { // setting the color of the card based on which for loop this is in - the reds will be filled, followed by the yellows, greens, and then blues
				strcpy(temp->suit, "Red\0");}
			else if (j == 1) {
				strcpy(temp->suit, "Yellow\0");}
			else if (j == 2) {
				strcpy(temp->suit, "Green\0");}
			else {
				strcpy(temp->suit, "Blue\0");}
			
			if (i == 10) { // seting up action cards if it is applicable, 0-9 don't get anything, but 10-13 have importance
				strcpy(temp->action, "Skip\0");}
			else if (i == 11) {
				strcpy(temp->action, "Reverse\0");}
			else if (i == 12) {
				strcpy(temp->action, "Draw Two\0");}
			else if (i == 13) {
				strcpy(temp->action, "Wild\0");}
			else { // setting 0-9 to have no statement
				strcpy(temp->action, "\0");}
		}
	}

	for (j = 0; j < 4; j++) { // doing the same thing again, except now there are no 0's and wild cards are wild draw four's
		for (i = 1; i < 14; i++) { // starts at 1 instead of 0
			temp = (card *)malloc(sizeof(card));
			if (head == NULL) {
				head = temp;
				tail = temp;
				tail->pt = NULL;
			}
			else {
				tail->pt = temp;
				tail = temp;
				tail->pt = NULL;
			}
			temp->value = i;
			if (j == 0) {
				strcpy(temp->suit, "Red\0");}
			else if (j == 1) {
				strcpy(temp->suit, "Yellow\0");}
			else if (j == 2) {
				strcpy(temp->suit, "Green\0");}
			else {
				strcpy(temp->suit, "Blue\0");}

			if (i == 10) {
				strcpy(temp->action, "Skip\0");}
			else if (i == 11) {
				strcpy(temp->action, "Reverse\0");}
			else if (i == 12) {
				strcpy(temp->action, "Draw Two\0");}
			else if (i == 13) { // wild is now wild draw four
				strcpy(temp->action, "Wild Draw Four\0");}
			else {
				strcpy(temp->action, "\0");}
		}
	}

	return head; // returns the head pointer to the entire list so that it can be used later

}
