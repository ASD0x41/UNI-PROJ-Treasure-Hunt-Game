// Muhammad Asad Tariq (21L-5266) --- BCS-3F
// Data Structures --- Assignment 2

#include <iostream>
#include <fstream>
#include <conio.h>
#include <string>
#include <time.h>

using namespace std;

//////////	SCAN CODES FOR ARROW KEYS	//////////

#define	U	72
#define	D	80
#define	L	75
#define	R	77

//////////	SCAN CODES FOR 'I', 'R' & 'X' KEYS	//////////

#define ITER	105			// I for running game iteratively
#define	RECR	114			// R for running game recursively
#define	EXIT	120			// X for exiting from the menu

//////////	ENUMERATED DATA TYPES CREATED FOR GAME	//////////

enum condition	// what condition is the player in? whether he is still searching for the treasure, or has something happened?
{
	HUNT_ONGOING,
	FOUND_TREASURE,
	HAD_BAD_LUCK,		// i.e. thrown out of the house by a portal
	EATEN_BY_DEVIL,
	PUNISHED_FOR_FOLLY	// i.e. used a portal after being sent to the treasure room by a gasper
};

enum travel		// type of movement made by the player
{
	NORMAL,
	TRANSFERRED_BY_PORTAL,
	RETURNED_BY_PORTAL,
	HELPED_BY_GASPER
};

//////////	CHARACTER BUFFER //////////

char trash = '\0';			// created to throw away commas when reading data of the form 'x, y' from files

//////////	DEFINITION OF CLASS TEMPLATE STACK	//////////

template <typename T>		// used for stacks of room pointers and steps of the path taken by the user through the house
class Stack					// stack implemented using a simple singly linked list without tail pointer or dummy nodes
{
private:

	struct Node
	{
		T data;

		Node* next;			// next* only, bcz singly linked list

		Node(const T& _data = NULL, Node* _next = NULL) : data(_data), next(_next) {}
	};

	Node* head;				// no need for tail ptr as stack only requires addToHead (push) and deleteFromHead (pop) functionality

public:

	Stack() : head(NULL) {}

	bool isEmpty()
	{
		return !head;
	}

	bool top(T& element)				// to obtain top element as pop() does not return it - it only removes it from the stack
	{
		if (head)
		{
			element = head->data;
			return true;				// the parameter passed by reference now represents the top element of the stack
		}
		else
			return false;				// returns false to indicate that the stack is empty, hence the referenced parameter is unchanged
	}

	void push(const T& element)			// is equivalent to the addToHead() functionality of a linked list
	{
		if (head)
			head = new Node(element, head);
		else
			head = new Node(element);
	}

	bool pop()							// is equivalent to the deleteFromHead() functionality of a linked list
	{
		if (head)
		{
			Node* temp = head;
			head = head->next;
			delete temp;
			return true;				// top element popped successfully
		}
		else
			return false;				// to indicate that the stack is empty hence popping is not possible
	}

	~Stack()
	{
		if (head)
		{
			Node* temp;
			do
			{
				temp = head;
				head = head->next;
				delete temp;
			} while (head);
		}
	}
};

/////////	DEFINITIONS OF STRUCTS AND CLASSES	//////////

struct Step
{
private:

	travel type;	// how did the player arrive at new position (i.e. via portal, gasper or normal movement)?
	int room;		// room the player is in

	int X;			// coordinates of new player position
	int Y;

public:

	Step(travel _type = NORMAL, int _room = 0, int x = 0, int y = 0) : type(_type), room(_room), X(x), Y(y) {}

	void show()		// to display each step at the end of the game when the path stack has to be displayed in reverse
	{
		switch (type)
		{
		case NORMAL:

			cout << "\tRoom # " << room << ": (" << X << ", " << Y << ")" << endl;
			break;

		case TRANSFERRED_BY_PORTAL:

			cout << "<Transferred to Room # " << room << " by a portal!>" << endl;
			break;

		case RETURNED_BY_PORTAL:

			cout << "<Returned to previous room by a portal!>" << endl;
			break;

		case HELPED_BY_GASPER:

			cout << "<Sent to the Treasure Room by a gasper!>" << endl;
		}
	}
};

class Room
{
private:

	const int ID;

	const int rows;
	const int cols;

	const int startRow;
	const int startCol;

	char** map;

	int rowPos;			// current/last position of player in room
	int colPos;

	bool treasure;		// does this room contain treasure

public:

	Room(int _ID, int len, int wid, ifstream& roomFile, int startR, int startC) : ID(_ID), rows(len), cols(wid), startRow(startR), startCol(startC)
	{
		map = new char*[rows];
		for (int i = 0; i < rows; i++)
			map[i] = new char[cols];

		treasure = false;

		char temp = '\0';
		for (int i = 0; i < rows; i++)
			for (int j = 0; j < cols; j++)		// loading map of room
			{
				roomFile >> temp;
				switch (temp)
				{
				case 'T':
					treasure = true;			// contains treasure

				case 'P':
				case 'G':
				case 'D':
				case 'X':

					map[i][j] = temp;
					break;

				default:

					map[i][j] = ' ';			// takes all other chars (except T, P, G, D and X) as representing empty traversable space
				}
			}

		switch (map[startRow][startCol])
		{
		case 'T':
		case 'P':
		case 'G':
		case 'D':
		case 'X':

			throw string("Invalid starting position specified in '");		// start position of player must be an empty space
		}

		SetStartPosition();
	}

	void SetStartPosition()		// bring player back to start position
	{
		rowPos = startRow;
		colPos = startCol;
	}

	int getID()
	{
		return ID;
	}

	bool hasTreasure() const
	{
		return treasure;
	}
	
	Step getPosition(int key = 0)	// to obtain a step object to be pushed into the path stack, based upon the movement made by the player
	{
		int i = rowPos, j = colPos;

		switch (key)
		{
		case U:

			i--;
			break;

		case D:

			i++;
			break;

		case L:

			j--;
			break;

		case R:

			j++;
			break;
		}

		return Step(NORMAL, ID, i, j);
	}

	char changePosition(int direction)		// returns whatever is found at the position that the player is trying to move to
	{
		char reached = '\0';	// to return null character if movement is invalid (e.g. out of room dimensions)

		int i = -1, j = -1;
		switch (direction)
		{
		case U:

			i = rowPos - 1;
			j = colPos;
			break;

		case D:

			i = rowPos + 1;
			j = colPos;
			break;

		case L:

			i = rowPos;
			j = colPos - 1;
			break;

		case R:

			i = rowPos;
			j = colPos + 1;
			break;
		}

		if ((i >= 0 && i < rows) && (j >= 0 && j < cols) && map[i][j] != 'X')
		{
			if (map[i][j] == ' ')		// if empty space, change player position in room as well
			{							// otherwise, do not change it so that player returns one step away from portal upon return to that room
				rowPos = i;
				colPos = j;
			}

			reached = map[i][j];	// return object found at new player position 
		}

		return reached;
	}
	
	void showMap() const		// display map of room (showing current position of player as well)
	{
		system("cls");

		cout << "ROOM #" << ID << endl << endl;
		for (int i = 0; i < rows; i++)
		{
			cout << "\t";
			for (int j = 0; j < cols; j++)
			{
				if (i == rowPos && j == colPos)
					cout << '*';	// * represents the player
				else
					cout << map[i][j];
				cout << ' ';
			}
			cout << endl;
		}
	}

	~Room()
	{
		for (int i = 0; i < rows; i++)
			delete[] map[i];
		delete[] map;
	}
};

class Game
{
private:

	Room** house;
	Room* treasureRoom;

	const int totalRooms;

public:

	Game(int _totalRooms, ifstream& houseFile) : totalRooms(_totalRooms)
	{
		house = new Room*[totalRooms];

		string fileName;
		ifstream roomFile;

		for (int i = 0; i < totalRooms; i++)	// to load maps of all rooms
		{
			house[i] = NULL;

			houseFile >> fileName;
			roomFile.open(fileName);

			if (roomFile.is_open())
			{
				int len = 0, wid = 0;
				roomFile >> len >> trash >> wid;

				if (len > 0 && wid > 0)		// the length and width of a room must be positive
				{
					int startRow = 0, startCol = 0;
					roomFile >> startRow >> trash >> startCol;

					if ((startRow >= 0 && startRow < len) && (startCol >= 0 && startCol < wid))		// the starting coordinates must lie within the room
					{
						try
						{
							house[i] = new Room(i + 1, len, wid, roomFile, startRow, startCol);		// corresponding room will be constructed
						}
						catch (string error)
						{
							throw string(error + fileName + "'!");		// in case of an error, it will be thrown back to main with the filename
						}
					}
					else
						throw string("Invalid starting position specified in '" + fileName + "'!");
				}
				else
					throw string("Invalid dimensions of room specified in '" + fileName + "'!");

				roomFile.close();
			}
			else
				throw string("Unable to open file '" + fileName + "'!");
		}

		treasureRoom = NULL;
		for (int i = 0; i < totalRooms && !treasureRoom; i++)
			if (house[i]->hasTreasure())
				treasureRoom = house[i];
		if (!treasureRoom)		// the house must contain a treausre room, otherwise the game will be unwinnable
			throw string("No room contains treasure!");
	}

	Room* getRandRoomExcept(Room* currentRoom) const	// returns pointer to a random room except the one given as argument
	{
		Room* randomRoom = house[0];

		if (totalRooms != 1)		// if there is only one room in the house then just return it, otherwise find another room
			do
			{
				randomRoom = house[rand() % totalRooms];
			} while (randomRoom == currentRoom);

		return randomRoom;
	}

	void ResetStartPositions()		// for playing again and again, reset the player position in each room
	{
		for (int i = 0; i < totalRooms; i++)
			house[i]->SetStartPosition();
	}
	
	void ITRGame()
	{								// iterative implmentation of game
		Stack<Step> path;
		Stack<Room*> roomStack;		// stack of room pointers instead of rooms to conserve space

		Room* currentRoom = getRandRoomExcept(NULL);	// i.e. return any random room
		roomStack.push(currentRoom);

		bool takenByGasper = false;	// if this flag is on, then player has to be punished if he uses portal

		condition outcome = HUNT_ONGOING;
		path.push(currentRoom->getPosition());	// push starting position to path stack

		int key = 0;
		while (outcome == HUNT_ONGOING)	// continue game unless something occurs to change outcome
		{
			currentRoom->showMap();		// display map after each movement

			key = _getch();
			if (key == U || key == D || key == L || key == R)
				switch (currentRoom->changePosition(key))
				{
				case 'T':

					path.push(currentRoom->getPosition(key));
					outcome = FOUND_TREASURE;
					break;

				case 'P':

					path.push(currentRoom->getPosition(key));
					if (!takenByGasper)
					{
						if (rand() % 2 == 1)	// head, i.e. move to a random room
						{
							roomStack.push(currentRoom);
							currentRoom = getRandRoomExcept(currentRoom);
							path.push(Step(TRANSFERRED_BY_PORTAL, currentRoom->getID()));
						}
						else     // tail, i.e. returned to previous room (or out of house if first room)
						{
							path.push(Step(RETURNED_BY_PORTAL));
							roomStack.top(currentRoom);
							roomStack.pop();
							if (roomStack.isEmpty())		// if returned from first room = kicked out of the house
								outcome = HAD_BAD_LUCK;
						}
					}
					else
						outcome = PUNISHED_FOR_FOLLY;
					break;

				case 'G':

					takenByGasper = true;
					path.push(currentRoom->getPosition(key));
					roomStack.push(currentRoom);
					currentRoom = treasureRoom;		// sent to treasure room by gasper
					path.push(Step(HELPED_BY_GASPER, currentRoom->getID()));
					break;

				case 'D':

					path.push(currentRoom->getPosition(key));
					outcome = EATEN_BY_DEVIL;
					break;

				case ' ':

					path.push(currentRoom->getPosition());	// just movement
				}
		}

		displayOutcome(outcome);	// display outcome of the game, as well as the path taken by the player
		displayPath(path);
		system("pause");
	}

	condition RECGame(Room* currentRoom, Stack<Step>& path, bool takenByGasper = false)
	{												// recursive implementation of game
		condition outcome = HUNT_ONGOING;
		path.push(currentRoom->getPosition());		// push starting position in room to path stack

		int key = 0;
		while (outcome == HUNT_ONGOING)		// keep running game unless outcome changes
		{
			currentRoom->showMap();		// display map of room (with player position)

			key = _getch();
			if (key == U || key == D || key == L || key == R)
				switch (currentRoom->changePosition(key))
				{
				case 'T':

					path.push(currentRoom->getPosition(key));
					outcome = FOUND_TREASURE;
					break;

				case 'P':

					path.push(currentRoom->getPosition(key));
					if (!takenByGasper)
					{
						if (rand() % 2 == 1)	// head
						{
							Room* nextRoom = getRandRoomExcept(currentRoom);
							path.push(Step(TRANSFERRED_BY_PORTAL, nextRoom->getID()));
							outcome = RECGame(nextRoom, path);	// recursive call for moving to random next room
						}
						else     // tail, return to previous room
						{
							path.push(Step(RETURNED_BY_PORTAL));
							outcome = HAD_BAD_LUCK;
						}
					}
					else
						outcome = PUNISHED_FOR_FOLLY;
					break;

				case 'G':

					path.push(currentRoom->getPosition(key));
					path.push(Step(HELPED_BY_GASPER, treasureRoom->getID()));
					outcome = RECGame(treasureRoom, path, true);	// taken to treasure room by gasper (recursive call for treasure room)
					break;

				case 'D':

					path.push(currentRoom->getPosition(key));
					outcome = EATEN_BY_DEVIL;
					break;

				case ' ':

					path.push(currentRoom->getPosition());
				}
		}

		if (outcome == HAD_BAD_LUCK)	// if about to be returned to previous room by portal, continue game in that room, don't exit house
			outcome = HUNT_ONGOING;		// but if previous room does not exit, will return to wrapper function, i.e. exit house

		return outcome;
	}

	void RECGame()		// wrapper for the recursive implementation of the game
	{
		Stack<Step> path;

		Room* startingRoom = getRandRoomExcept(NULL);	// start from any random room

		condition outcome = RECGame(startingRoom, path); // run game and see outcome

		displayOutcome(outcome);		// display outcome when it ends
		displayPath(path);				// display path taken by player
		system("pause");
	}
	
	void reversePrint(Stack<Step>& path)		// recursive function to print path stack in reverse
	{
		Step temp;
		while (path.top(temp))
		{
			path.pop();
			reversePrint(path);
			temp.show();
		}
	}

	void displayPath(Stack<Step>& path)		// path taken by the user through the house
	{
		cout << "Your path through the haunted house:" << endl;
		reversePrint(path);
	}

	void displayOutcome(condition outcome)	  // dispay message according to the outcome of the game
	{
		system("cls");

		switch (outcome)
		{
		case FOUND_TREASURE:

			cout << "Congratulations! You found the treasure!" << endl;
			break;

		case HAD_BAD_LUCK:

			cout << "Unfortunately, the portal kicked you out of the house." << endl;
			break;

		case EATEN_BY_DEVIL:

			cout << "Sadly, you were eaten by a devil." << endl;
			break;

		case PUNISHED_FOR_FOLLY:

			cout << "A gasper took you to the treasure room. Why on earth did you use the portal then?" << endl;
			break;
		}
	}

	~Game()
	{
		for (int i = 0; i < totalRooms; i++)
			delete house[i];
		delete[] house;
	}
};

///////////	MAIN : DRIVER PROGRAM	//////////

void main()
{
	srand(time(NULL));

	try
	{
		string fileName = "hauntedhouse.txt";
		ifstream houseFile(fileName);

		if (houseFile.is_open())
		{
			int totalRooms = 0;
			houseFile >> totalRooms;

			if (totalRooms > 0)		// no. of rooms must be positive
			{
				Game hauntedHouse(totalRooms, houseFile);		// filestream passed to constrcutor of game to load the maps of the rooms

				int choice = 0;
				while (choice != EXIT)
				{
					system("cls");

					cout << "Welcome to the 'Treasure Hunt in a Haunted House' game!" << endl;
					cout << "Press I or R to play the game, or X to exit from the game." << endl;
					cout << "(The game will run using iteration for I and recursion for R.)" << endl;

					hauntedHouse.ResetStartPositions();		// needed in case user wants to play the game again

					switch (choice = _getch())
					{
					case ITER:

						hauntedHouse.ITRGame();
						break;

					case RECR:

						hauntedHouse.RECGame();
						break;

					case EXIT:

						cout << endl << "You have chosen to exit the game. Thanks for playing!" << endl;
						break;
					}
				}
			}
			else
				throw string("Invalid number of rooms specified in '" + fileName + "'!");

			houseFile.close();
		}
		else
			throw string("Unable to open file '" + fileName + "'!");
	}

	catch (string error)	// to display error strings thrown as exceptions from any part of the game loading process
	{
		cout << "Error: " << error << endl;
	}

	system("pause");
}