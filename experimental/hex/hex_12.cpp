// Hex Maniac v1.2
// Copyright Cameron Browne
// 5/10/2000
//
// Point-pairing algorithm with blind row.
//
// Feel free to experiment with and develop this code, but please 
// acknowledge its contribution to any derived work. 
//
// Compiles with the Microsoft C/C++ compiler (12.00.8168) and linker 
// (6.00.8168) but should be portable. Use the following options:
//     cl -GX hex_12.cpp 
//
// Guaranteed bug free. Any errors must have been introduced by 
// subsequent developers :)

#include <iostream>

using std::cout;
using std::cin;
using std::endl;


//////////////////////////////////////////////////////////////////////////////

const int MIN_N = 3;        // Smallest board size
const int MAX_N = 26;	    // Largest board size (limit of alphanumeric notation)

static enum State	          // board point or move state
{	
    EMPTY = 0,            // empty board position
    VERT  = 1,            // computer player (vertical = j)
    HORZ  = 2,            // human player (horizontal = i)
    NUM_STATES
};

static char* StateNames[NUM_STATES] =
{
    "Empty",
    "Vertical",
    "Horizontal",
};

struct Coord 
{ 
	Coord(int i_=-1, int j_=-1) : i(i_), j(j_) {}

	int i; 
	int j; 
};

// Hexagonal neighbours of X:
//             >-<                   
//          >-< 0 >-<          | 5 | 0
//         < 5 >-< 1 >      ---+---+---
//          >-< X >-<   ==   4 | X | 1
//         < 4 >-< 2 >      ---+---+---
//          >-< 3 >-<        3 | 2 |
//             >-< 
static const Coord nbor[6] = 
{ 
    Coord(1,-1), Coord(1,0),  Coord(0,1), 
    Coord(-1,1), Coord(-1,0), Coord(0,-1)
};

// Blind row defense templates
//
// Key:
//     h = existing HORZ piece
//     v = existing VERT piece
//     . = empty position
//     x = existing piece (VERT or HORZ)
//     - = don't care
//     H = last HORZ move
//     V = VERT's best reply
//     r = reentrant block on furthest connected piece (or adjacent if none)
//
// Note: more general situations should go last.
//
const int NUM_DEFENSE_TEMPLATES = 25;
static const char* HEX_DefenseTemplate[NUM_DEFENSE_TEMPLATES][2] =
{
	{ "H V - -", "- - - -" },	// acute blind corner
	{ "- - . H", "- - V -" },	// obtuse blind corner
	{ "- V H v", "- - - -" },
	{ "- v H V", "- - - -" },
	{ "- V H .", "h . . h" },
	{ "- . H V", "h . . h" },
	{ "v h H -", "V . h -" },
	{ "- H h v", "h . V -" },
	{ "- V H -", "h . h -" },
	{ "- - H V", "- h . h" },
	{ "- V H -", "- v - -" },
	{ "- H V -", "- v - -" },
	{ "- V H -", "- - h -" },
	{ "- H V -", "h - - -" },
	{ ". H h -", "V - - -" },
	{ "- h H .", "- - V -" },
	{ "- V H -", "h . - -" },
	{ "- H V -", "- . h -" },
	{ "- h H h", "- v V v" },
	{ "- h H h", "v V v -" },
	{ "- h H v", "- v V -" },
	{ "- v H h", "- V v -" },
	{ "- H r -", "h - - -" },
	{ "- r H -", "- - h -" },
	{ "- . H .", "- V . -" },	// reentrant block below (general case)
};


///////////////////////////////////////////////////////////////////////////////

class HEX_Game
{
public:
	HEX_Game(int n_=11) : N(n_), SwapOption(true) {}
	~HEX_Game()	{}

	bool	Play(void);
private:
	bool	DoMove(State who);
	bool	GetUserMove(Coord& move) const;
	bool	GameWon(State who);
	bool	GameWon(State who, Coord const& from);
	void	RandomEmptyPosition(Coord& posn) const;
	void	ShowBoard(void) const;
	bool	IsValid(Coord const& posn) const;

	void	BestOpening(Coord& move) const;
	bool	GoodSwap(Coord const& move) const;
	bool	BestComputerMove(Coord& move) const;
	bool	DefendBlindRow(Coord& move) const;
	void	SpareMove(Coord& move) const;

private:
    State   Board[MAX_N][MAX_N];   // the board
    int     N;                     // current board size NxN
    int     NumMoves;              // number of moves played
    Coord   Last;                  // opponent's last move
    bool    SwapOption;            // whether the swap option is enabled
    bool    HasSwapped;            // whether a player has swapped already
    State   WhoStarted;            // who played first
    bool    Visit[MAX_N][MAX_N];   // for rough working
};


///////////////////////////////////////////////////////////////////////////////

// Alternates moves between players until the game is won
//
// Returns: whether user choose to continue playing
//
bool HEX_Game::Play(void)	
{
	memset(Board, 0, sizeof(Board));
	NumMoves = 0;
	HasSwapped = false;

	// Determine who starts
	cout << "\nDo you want to start? [y/n/q]: ";
	char ch('n');
	cin >> ch;
		
	if (tolower(ch) == 'q')	
		exit(1);

	WhoStarted = (tolower(ch) == 'y') ? HORZ : VERT;
	
	if (WhoStarted == HORZ)
		ShowBoard();

	// Play a game
	State who(WhoStarted);
	while (!DoMove(who))
		who = State((NumMoves + WhoStarted + 1) % 2 + 1);

	cout << "\nGame won by " << StateNames[who] << "!" << endl;
	cout << "\nPlay again on " << N << 'x' << N << "? [y/n]: ";
	cin >> ch;
	
	return (tolower(ch) == 'y'); 
}

// Determines and plays a move for the specfied player
//
// Returns: whether this move wins the game
//
bool HEX_Game::DoMove(State who)
{
	Coord move;
	bool did_swap = (who==HORZ) ? GetUserMove(move) : BestComputerMove(move);
	if (did_swap)
	{
		// 'who' chose to swap
		State opp = (who == VERT) ? HORZ : VERT;
		Board[Last.j][Last.i] = EMPTY;	// undo opening move
		Board[Last.i][Last.j] = who;	// redo its reflection
		ShowBoard();
		cout << endl << "Swap! " << StateNames[who][0] << " takes ";
		cout << char('A' + Last.j) << 1 + Last.i << "." << endl;

		WhoStarted = who;
		HasSwapped = true;
	}
	else
	{
		// Make the move
		Board[move.j][move.i] = who;
		NumMoves++;
		Last = move;

		// Show the result
		ShowBoard();
		cout << endl << StateNames[who][0] << " plays at ";
		cout << char('A' + move.i) << 1 + move.j << "." << endl;
	}

	return GameWon(who);
}

// Reads user's move from the keyboard (HORZ)
//
// Returns: whether user chose to swap
//
bool HEX_Game::GetUserMove(Coord& move) const
{
	char str[4];
	if (SwapOption && !HasSwapped && NumMoves==1)
	{
		char ch('y');
		cout << endl << "Swap opening move? "; 
		if (GoodSwap(Last))
			cout << "(I would) [y/n/q]: ";
		else
			cout << "(I wouldn't) [y/n/q]: ";
		cin >> ch;
		
		if (tolower(ch) == 'q')	
			exit(2);

		if (tolower(ch) == 'y')
			return true;		// Swap
	}

	do {
		cout << endl << "Enter move [eg f6/q]: ";
		cin >> str;

		if (str[0] == 'q')
			exit(3);		

		move.i = tolower(str[0]) - 'a';
		move.j = atoi(&str[1]) - 1;
	} while (!IsValid(move) || Board[move.j][move.i]);

	return false;
}

// Returns: whether the game has been won by the specified player
//
bool HEX_Game::GameWon(State who)
{
	memset(Visit, 0, sizeof(Visit));
	if (who == VERT)
	{
		for (int i = 0; i < N; i++)
			if (Board[0][i]==VERT && !Visit[0][i] && GameWon(VERT, Coord(i,0)))
				return true;
	}
	else
	{
		for (int j = 0; j < N; j++)
			if (Board[j][0]==HORZ && !Visit[j][0] && GameWon(HORZ, Coord(0,j)))
				return true;
	}
	return false;
}

// Recursively examines neighbours of potential winning chains
//
// Returns: whether the game has been won by the specified player
//
bool HEX_Game::GameWon(State who, Coord const& from)
{
	if (who==HORZ && from.i==N-1 || who==VERT && from.j==N-1)
		return true;	

	Visit[from.j][from.i] = true;		// this coord has been visited

	for (int n = 0; n < 6; n++)
	{ 
		// Check neighbours for continuation of chain
		Coord to(from.i + nbor[n].i, from.j + nbor[n].j);
		if 
		(
			IsValid(to)	&& Board[to.j][to.i]==who 
			&& 
			!Visit[to.j][to.i] && GameWon(who, to)
		)
			return true;	
	}
	return false;
}

// Draws board in ASCII text format
//
void HEX_Game::ShowBoard(void) const
{
	//	Show alphabetical (HORZ) labels
	cout << endl << "  ";
	for (int i = 0; i < N; i++)
		cout << char('A' + i) << ' ';
	cout << endl;

	for (int j = 0; j < N; j++)
	{
		// Show numeric (VERT) labels
		for (int s = 0; s < j; s++)
			cout << ' ';
		if (j < 9)
			cout << ' ';
		cout << j + 1;

		// Show state of each board position along this row
		for (int i = 0; i < N; i++)
			if (Board[j][i] == VERT)
				cout << " V";
			else if (Board[j][i] == HORZ)
				cout << " H";
			else 
				cout << " .";
		cout << endl;
	}
}

// Returns: whether posn is within the bounds of the board
//
bool	
HEX_Game::IsValid(Coord const& posn) const
{
	return (posn.i >= 0 && posn.i < N && posn.j >= 0 && posn.j < N);
}


///////////////////////////////////////////////////////////////////////////////
// Strategic operations

// Determines computer's best opening move (VERT)
//
void HEX_Game::BestOpening(Coord& move) const
{
	move = Coord(N - 1, 0);					// obtuse corner in blind row
}
	
// Returns: whether this opening move is worth swapping
//
bool	
HEX_Game::GoodSwap(Coord const& move) const
{
	return (move.i == N - move.j - 1);		// swap along short diagonal
}

// Determines best move for computer (VERT)
//
// Returns:  true on swap, else false
//
bool 
HEX_Game::BestComputerMove(Coord& move) const
{	
	if (NumMoves == 0)
	{
		BestOpening(move);				// opening move
	}
	else if (NumMoves == 1 && SwapOption && !HasSwapped && GoodSwap(Last))
	{
		return true;					// swap opponent's opening move
	}
	else if (Last.j == 0)
	{
		DefendBlindRow(move);			// defend the blind row	
	}
	else
	{
		if (Last.i < N - Last.j)		// determine dual point on left
			move = Coord(N - Last.j, N - Last.i - 1);
		else							// determine dual point on right
			move = Coord(N - Last.j - 1, N - Last.i);
	}

	if (!IsValid(move) || Board[move.j][move.i])
		SpareMove(move);				// spare move - do some damage!

	return false;
}

// Determines best move along the blind row (row 0)
//
// Returns:
//     Whether appropriate defending move was found.
//
bool HEX_Game::DefendBlindRow(Coord& move) const
{
	move = Coord(-1, -1);		// should already be initialised but make sure
	
	for (int d = 0; d < NUM_DEFENSE_TEMPLATES; d++)
	{
		// Find root position H on top row
		for (int root = 0; root < 4; root++)
			if (HEX_DefenseTemplate[d][0][root * 2] == 'H')
				break;
		if 
		(
			root>=4									// no root found
			||
			root==0 && !(Last.i==0 && Last.j==0)	// acute corner no good	
			||
			root==3 && !(Last.i==N-1 && Last.j==0)	// obtuse corner no good
		)
			continue;

		// Check whether rest of template matches board position
		bool valid		= true;
	
		int	reentrant	= 0;

		for (int j = 0; j < 2 && valid; j++)
			for (int c = 0; c < 4 && valid; c++)
			{
				int i = Last.i - root + c;
				char ch = HEX_DefenseTemplate[d][j][c * 2];

				if (ch == 'V' && !Board[j][i])				
					move = Coord(i, j);
				else if (ch == 'V' && Board[j][i])
					valid = false;
				else if (ch == 'h' && Board[j][i] != HORZ)
					valid = false;
				else if (ch == 'v' && Board[j][i] != VERT)
					valid = false;
				else if (ch == 'x' && !Board[j][i])
					valid = false;
				else if (ch == '.' && Board[j][i])
					valid = false;
				else if (ch == 'r')
				{
					if (Board[j][i] == VERT)
						valid = false;			
					else
					{
						move = Coord(Last);
						reentrant = (i < root) ? -1 : 1;  // is reentrant move
					}
				}
			}

		if (!valid)
			continue;		// invalid template

		if (move.i == -1 || move.j == -1)
			continue;		// no valid reply found

		if (reentrant)
		{
			if (reentrant > 0)
			{
				// Find reentrant block above
				while (move.i < N - 1)
				{
					if (!Board[0][move.i])
						return true;			// adjacent block above
					
					if (!Board[1][move.i])
					{
						move = Coord(move.i, 1);
						return true;			// reentrant block above
					}
					move.i++;
				}
			}
			else
			{
				// Find reentrant block below
				while (move.i >= 0)
				{
					if (!Board[0][move.i])
						return true;			// adjacent block above
					
					if (!Board[1][move.i - 1])	
					{
						move = Coord(move.i - 1, 1);
						return true;			// reentrant block above
					}
					move.i--;
				}
			}
			return false;						//	no reentrant block found
		}
		return  true;	// good move found!
	}
	return false;
}


// Determines best spare move. There will ALWAYS be a spare move somewhere.	
//
void HEX_Game::SpareMove(Coord& move) const
{
	// Look for urgent moves on the blind row
	move.j = 0;
	for (move.i = 0; move.i < N; move.i++)
		if 
		(
			!Board[0][move.i] 
			&&
			(
				move.i < N - 1 && Board[0][move.i + 1] == HORZ 
				&& 
				(
					Board[1][move.i] == VERT 
					||
					move.i > 0 && !Board[1][move.i] && Board[1][move.i-1]==HORZ
				)
				||
				move.i > 0 && Board[0][move.i - 1] == HORZ 
				&& 
				(
					Board[1][move.i - 1] == VERT 
					||
					!Board[1][move.i] && Board[1][move.i + 1] == HORZ 
				)
			)
		)
			return;

	// If top obtuse corner is empty, take it
	if (!Board[0][N - 1])
	{
		move = Coord(N - 1, 0);
		return;
	}

	// Block any reentrant point on row 1
	move.j = 1;
	for (move.i = 0; move.i < N - 1; move.i++)		// skip move.i == N - 1
		if 
		(
			!Board[1][move.i] 
			&&
			(
				Board[0][move.i] == HORZ && Board[0][move.i + 1] != HORZ 
				||
				Board[0][move.i + 1] == HORZ && Board[0][move.i] != HORZ
			)
		)
			return;

	// Block any point adjacent to White on row 0
	move.j = 0;
	for (move.i = 0; move.i < N; move.i++)
		if 
		(
			!Board[0][move.i] 
			&&
			(
				move.i > 0 && Board[0][move.i - 1] == HORZ 
				||
				move.i < N - 1 && Board[0][move.i + 1] == HORZ
			)
		)
			return;

	// Take any point an empty point away from Black on row 0
	move.j = 0;
	for (move.i = 0; move.i < N; move.i++)
		if 
		(
			!Board[0][move.i] 
			&&
			(
				move.i > 1 && !Board[0][move.i-1] && Board[0][move.i-2]==VERT 
				||
				move.i < N-2 && !Board[0][move.i+1] && Board[0][move.i+2]==VERT 
			)
		)
			return;

	// Take any empty point from top obtuse corner down (must be at least one)
	for (move.j = 0; move.j < N; move.j++)
		for (move.i = N-1; move.i >= 0; move.i--)
			if (!Board[move.j][move.i])
				return;

	move = Coord(-1, -1);	// bad result
} 	 	


///////////////////////////////////////////////////////////////////////////////

void main(int argc, char* argv[])
{
	cout << "Hex Maniac v1.2" << endl;
	cout << "Cameron Browne 5/10/2000" << endl << endl;
	cout << "You are horizontal (H), computer is vertical (V)" << endl;
	
	int n = 11;
	if (argc == 2)
	{
		n = atoi(argv[1]);
		if (n < MIN_N)	
			n = MIN_N;
		else if (n > MAX_N)	
			n = MAX_N;
	}

	HEX_Game game(n);
	while (game.Play());
}
