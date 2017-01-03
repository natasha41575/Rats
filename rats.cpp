// rats.cpp

// We've provided some incorrect return statements (so indicated) just
// to allow this skeleton program to compile and run, albeit incorrectly.
// The first thing you probably want to do is implement the utterly trivial
// functions (marked TRIVIAL).  Then get Arena::display going.  That gives
// you more flexibility in the order you tackle the rest of the functionality.

#include <iostream>
#include <string>
#include <random>
#include <utility>
#include <cstdlib>
#include <cctype>
using namespace std;

///////////////////////////////////////////////////////////////////////////
// Manifest constants
///////////////////////////////////////////////////////////////////////////

const int MAXROWS = 20;            // max number of rows in the arena
const int MAXCOLS = 20;            // max number of columns in the arena
const int MAXRATS = 100;           // max number of rats allowed

const int NORTH = 0;
const int EAST  = 1;
const int SOUTH = 2;
const int WEST  = 3;
const int NUMDIRS = 4;

const int EMPTY      = 0;
const int HAS_POISON = 1;

///////////////////////////////////////////////////////////////////////////
// Type definitions
///////////////////////////////////////////////////////////////////////////

class Arena;  // This is needed to let the compiler know that Arena is a
              // type name, since it's mentioned in the Rat declaration.

class Rat
{
  public:
      // Constructor
    Rat(Arena* ap, int r, int c);

      // Accessors
    int  row() const;
    int  col() const;
    bool isDead() const;

      // Mutators
    void move();

  private:
    Arena* m_arena;
    int    m_row;
    int    m_col;
    int    m_pellets; //number of pellets rat has eaten
    int    m_turn; //variable used to keep track of when a rat that has been poisoned should move
};

class Player
{
  public:
      // Constructor
    Player(Arena* ap, int r, int c);

      // Accessors
    int  row() const;
    int  col() const;
    bool isDead() const;

      // Mutators
    string dropPoisonPellet();
    string move(int dir);
    void   setDead();

  private:
    Arena* m_arena;
    int    m_row;
    int    m_col;
    bool   m_dead;
};

class Arena
{
  public:
      // Constructor/destructor
    Arena(int nRows, int nCols);
    ~Arena();

      // Accessors
    int     rows() const;
    int     cols() const;
    Player* player() const;
    int     ratCount() const;
    int     getCellStatus(int r, int c) const;
    int     numberOfRatsAt(int r, int c) const;
    void    display(string msg) const;

      // Mutators
    void setCellStatus(int r, int c, int status);
    bool addRat(int r, int c);
    bool addPlayer(int r, int c);
    void moveRats();

  private:
    int     m_grid[MAXROWS][MAXCOLS];
    int     m_rows;
    int     m_cols;
    Player* m_player;
    Rat*    m_rats[MAXRATS];
    int     m_nRats;
    int     m_turns;

      // Helper functions
    void checkPos(int r, int c) const;
    bool isPosInBounds(int r, int c) const;
};

class Game
{
  public:
      // Constructor/destructor
    Game(int rows, int cols, int nRats);
    ~Game();

      // Mutators
    void play();

  private:
    Arena* m_arena;

      // Helper functions
    string takePlayerTurn();
};

///////////////////////////////////////////////////////////////////////////
//  Auxiliary function declarations
///////////////////////////////////////////////////////////////////////////

int randInt(int min, int max);
bool decodeDirection(char ch, int& dir);
bool attemptMove(const Arena& a, int dir, int& r, int& c);
bool recommendMove(const Arena& a, int r, int c, int& bestDir);
void clearScreen();
int check_rats(const Arena& a, int r, int c);

///////////////////////////////////////////////////////////////////////////
//  Rat implementation
///////////////////////////////////////////////////////////////////////////

Rat::Rat(Arena* ap, int r, int c)
{
    if (ap == nullptr)
    {
        cout << "***** A rat must be created in some Arena!" << endl;
        exit(1);
    }
    if (r < 1  ||  r > ap->rows()  ||  c < 1  ||  c > ap->cols())
    {
        cout << "***** Rat created with invalid coordinates (" << r << ","
             << c << ")!" << endl;
        exit(1);
    }
    m_arena = ap;
    m_row = r;
    m_col = c;
    m_turn = 0; //keeps track of when the rat will move 
    m_pellets = 0; //keeps track of how many pellets the rat has eaten
}

int Rat::row() const
{
    return m_row;
}

int Rat::col() const
{
    return m_col;
}

bool Rat::isDead() const
{
    return m_pellets > 1;
}

void Rat::move()
{
      //   Return without moving if the rat has eaten one poison pellet (so
      //   is supposed to move only every other turn) and this is a turn it
      //   does not move.
    if (m_pellets == 1 && m_turn%2==1){
       m_turn++;
       return;
    }
 
      //   Otherwise, attempt to move in a random direction; if it can't
      //   move, don't move.  If it lands on a poison pellet, eat the pellet,
      //   so it is no longer on that grid point.
    int dir = randInt(0, NUMDIRS-1);
    attemptMove(*m_arena, dir, m_row, m_col);
    m_turn++;  

    if (m_arena->getCellStatus(m_row, m_col) == HAS_POISON){
        m_pellets++;
        m_turn = 1;
    }  
    m_arena->setCellStatus(m_row, m_col, EMPTY);
}

///////////////////////////////////////////////////////////////////////////
//  Player implementation
///////////////////////////////////////////////////////////////////////////

Player::Player(Arena* ap, int r, int c)
{
    if (ap == nullptr)
    {
        cout << "***** The player must be created in some Arena!" << endl;
        exit(1);
    }
    if (r < 1  ||  r > ap->rows()  ||  c < 1  ||  c > ap->cols())
    {
        cout << "**** Player created with invalid coordinates (" << r
             << "," << c << ")!" << endl;
        exit(1);
    }
    m_arena = ap;
    m_row = r;
    m_col = c;
    m_dead = false;
}

int Player::row() const
{
    return m_row;
}

int Player::col() const
{
    return m_col;
}

string Player::dropPoisonPellet()
{
    if (m_arena->getCellStatus(m_row, m_col) == HAS_POISON)
        return "There's already a poison pellet at this spot.";
    m_arena->setCellStatus(m_row, m_col, HAS_POISON);
    return "A poison pellet has been dropped.";
}

string Player::move(int dir)
{
      // Attempt to move the player one step in the indicated
      //        direction.  If this fails,
      //        return "Player couldn't move; player stands."
      //        A player who moves onto a rat dies, and this
      //        returns "Player walked into a rat and died."
      //        Otherwise, return one of "Player moved north.",
      //        "Player moved east.", "Player moved south.", or
      //        "Player moved west."
    
    bool p_moved = attemptMove(*m_arena, dir, m_row, m_col);

    if (!p_moved)
        return "Player couldn't move; player stands.";

    if (m_arena->numberOfRatsAt(m_row, m_col) > 0){
        this->setDead();
        return "Player walked into a rat and died.";
    }

    if (dir == NORTH)
        return "Player moved north.";
    else if (dir == EAST)
        return "Player moved east.";
    else if (dir == SOUTH)
        return "Player moved south.";
    else 
        return "Player moved west.";
}

bool Player::isDead() const
{
    return m_dead;
}

void Player::setDead()
{
    m_dead = true;
}

///////////////////////////////////////////////////////////////////////////
//  Arena implementation
///////////////////////////////////////////////////////////////////////////

Arena::Arena(int nRows, int nCols)
{
    if (nRows <= 0  ||  nCols <= 0  ||  nRows > MAXROWS  ||  nCols > MAXCOLS)
    {
        cout << "***** Arena created with invalid size " << nRows << " by "
             << nCols << "!" << endl;
        exit(1);
    }
    m_rows = nRows;
    m_cols = nCols;
    m_player = nullptr;
    m_nRats = 0;
    m_turns = 0;
    for (int r = 1; r <= m_rows; r++)
        for (int c = 1; c <= m_cols; c++)
            setCellStatus(r, c, EMPTY);
}

Arena::~Arena()
{
    delete m_player;

    for (int i = 0; i < m_nRats; i++)
        delete m_rats[i];
}

int Arena::rows() const
{
    return m_rows;
}

int Arena::cols() const
{
    return m_cols;
}

Player* Arena::player() const
{
    return m_player;
}

int Arena::ratCount() const
{
    return m_nRats;
}

int Arena::getCellStatus(int r, int c) const
{
    checkPos(r, c);
    return m_grid[r-1][c-1];
}

int Arena::numberOfRatsAt(int r, int c) const
{
    int rat_count = 0;

    for (int i = 0; i < m_nRats; i++){
      if (m_rats[i]->row() == r && m_rats[i]->col() == c)
        rat_count++;
    }

    return rat_count;
}

void Arena::display(string msg) const
{
    char displayGrid[MAXROWS][MAXCOLS];
    int r, c;
    
      // Fill displayGrid with dots (empty) and stars (poison pellets)
    for (r = 1; r <= rows(); r++)
        for (c = 1; c <= cols(); c++)
            displayGrid[r-1][c-1] = (getCellStatus(r,c) == EMPTY ? '.' : '*');

      // Indicate each rat's position
      // If one rat is at some grid point, set the displayGrid char
      //        to 'R'.  If it's 2 though 8, set it to '2' through '8'.
      //        For 9 or more, set it to '9'.

    for (r = 1; r <= rows(); r++)
        for (c = 1; c <= cols(); c++){
            if (numberOfRatsAt(r, c) == 1)
                displayGrid[r-1][c-1] = 'R';
            else if (numberOfRatsAt(r, c) >= 2 && numberOfRatsAt(r, c) <= 8)
                displayGrid[r-1][c-1] = numberOfRatsAt(r, c) + '0';
            else if (numberOfRatsAt(r, c) > 9)
                displayGrid[r-1][c-1] = '9';     
    }

      // Indicate player's position
    if (m_player != nullptr)
        displayGrid[m_player->row()-1][m_player->col()-1] = (m_player->isDead() ? 'X' : '@');

      // Draw the grid
    clearScreen();
    for (r = 1; r <= rows(); r++)
    {
        for (c = 1; c <= cols(); c++)
            cout << displayGrid[r-1][c-1];
        cout << endl;
    }
    cout << endl;

      // Write message, rat, and player info
    if (msg != "")
        cout << msg << endl;
    cout << "There are " << ratCount() << " rats remaining." << endl;
    if (m_player == nullptr)
        cout << "There is no player!" << endl;
    else if (m_player->isDead())
        cout << "The player is dead." << endl;
    cout << m_turns << " turns have been taken." << endl;
}

void Arena::setCellStatus(int r, int c, int status)
{
    checkPos(r, c);
    m_grid[r-1][c-1] = status;
}

bool Arena::addRat(int r, int c)
{
    if (! isPosInBounds(r, c))
        return false;

      // Don't add a rat on a spot with a poison pellet
    if (getCellStatus(r, c) != EMPTY)
        return false;

      // Don't add a rat on a spot with a player
    if (m_player != nullptr  &&  m_player->row() == r  &&  m_player->col() == c)
        return false;

      // If there are MAXRATS existing rats, return false.  Otherwise,

    if (m_nRats >= MAXRATS) 
        return false;

      // dynamically allocate a new rat at coordinates (r,c).  Save the
      // pointer to the newly allocated rat and return true.

    m_rats[m_nRats] = new Rat(this, r, c);
    m_nRats++;
    return true;
}

bool Arena::addPlayer(int r, int c)
{
    if (! isPosInBounds(r, c))
        return false;

      // Don't add a player if one already exists
    if (m_player != nullptr)
        return false;

      // Don't add a player on a spot with a rat
    if (numberOfRatsAt(r, c) > 0)
        return false;

    m_player = new Player(this, r, c);
    return true;
}

void Arena::moveRats()
{
      // Move all rats
      // Move each rat.  Mark the player as dead if necessary.
      //        Deallocate any dead dynamically allocated rat.

  for (int i = 0; i < m_nRats; i++){  
      m_rats[i]->move();

      if (m_player->row() == m_rats[i]->row() && m_player->col() == m_rats[i]->col())
          m_player->setDead();

      if (m_rats[i]->isDead()){
          delete m_rats[i];
          for (int j = i+1; j < m_nRats; j++) //move the remaining rats one to the left in the array
              m_rats[j-1] = m_rats[j];
          m_nRats--; 
      } 
  }

      // Another turn has been taken
    m_turns++;
}

bool Arena::isPosInBounds(int r, int c) const
{
    return (r >= 1  &&  r <= m_rows  &&  c >= 1  &&  c <= m_cols);
}

void Arena::checkPos(int r, int c) const
{
    if (r < 1  ||  r > m_rows  ||  c < 1  ||  c > m_cols)
    {
        cout << "***** " << "Invalid arena position (" << r << ","
             << c << ")" << endl;
        exit(1);
    }
}

///////////////////////////////////////////////////////////////////////////
//  Game implementation
///////////////////////////////////////////////////////////////////////////

Game::Game(int rows, int cols, int nRats)
{
    if (nRats < 0)
    {
        cout << "***** Cannot create Game with negative number of rats!" << endl;
        exit(1);
    }
    if (nRats > MAXRATS)
    {
        cout << "***** Trying to create Game with " << nRats
             << " rats; only " << MAXRATS << " are allowed!" << endl;
        exit(1);
    }
    if (rows == 1  &&  cols == 1  &&  nRats > 0)
    {
        cout << "***** Cannot create Game with nowhere to place the rats!" << endl;
        exit(1);
    }

      // Create arena
    m_arena = new Arena(rows, cols);

      // Add player
    int rPlayer;
    int cPlayer;
    do
    {
        rPlayer = randInt(1, rows);
        cPlayer = randInt(1, cols);
    } while (m_arena->getCellStatus(rPlayer, cPlayer) != EMPTY);
    m_arena->addPlayer(rPlayer, cPlayer);

      // Populate with rats
    while (nRats > 0)
    {
        int r = randInt(1, rows);
        int c = randInt(1, cols);
        if (r == rPlayer && c == cPlayer)
            continue;
        m_arena->addRat(r, c);
        nRats--;
    }
}

Game::~Game()
{
    delete m_arena;
}

string Game::takePlayerTurn()
{
    for (;;)
    {
        cout << "Your move (n/e/s/w/x or nothing): ";
        string playerMove;
        getline(cin, playerMove);

        Player* player = m_arena->player();
        int dir;

        if (playerMove.size() == 0)
        {
            if (recommendMove(*m_arena, player->row(), player->col(), dir))
                return player->move(dir);
            else
                return player->dropPoisonPellet();
        }
        else if (playerMove.size() == 1)
        {
            if (tolower(playerMove[0]) == 'x')
                return player->dropPoisonPellet();
            else if (decodeDirection(playerMove[0], dir))
                return player->move(dir);
        }
        cout << "Player move must be nothing, or 1 character n/e/s/w/x." << endl;
    }
}

void Game::play()
{
    m_arena->display("");
    while ( ! m_arena->player()->isDead()  &&  m_arena->ratCount() > 0)
    {
        string msg = takePlayerTurn();
        Player* player = m_arena->player();
        if (player->isDead())
        {
            cout << msg << endl;
            break;
        }
        m_arena->moveRats();
        m_arena->display(msg);
    }
    if (m_arena->player()->isDead())
        cout << "You lose." << endl;
    else
        cout << "You win." << endl;
}

///////////////////////////////////////////////////////////////////////////
//  Auxiliary function implementation
///////////////////////////////////////////////////////////////////////////

  // Return a uniformly distributed random int from min to max, inclusive
int randInt(int min, int max)
{
    if (max < min)
        swap(max, min);
    static random_device rd;
    static mt19937 generator(rd());
    uniform_int_distribution<> distro(min, max);
    return distro(generator);
}

bool decodeDirection(char ch, int& dir)
{
    switch (tolower(ch))
    {
      default:  return false;
      case 'n': dir = NORTH; break;
      case 'e': dir = EAST;  break;
      case 's': dir = SOUTH; break;
      case 'w': dir = WEST;  break;
    }
    return true;
}

  // Return false without changing anything if moving one step from (r,c)
  // in the indicated direction would run off the edge of the arena.
  // Otherwise, update r and c to the position resulting from the move and
  // return true.
bool attemptMove(const Arena& a, int dir, int& r, int& c)
{
    int row = r;
    int col = c;

    if (dir == NORTH)
        row--;
    else if (dir == SOUTH)
        row++;
    else if (dir == WEST)
        col--;
    else if (dir == EAST)
        col++;

    //only change if the position is within the arena
    if (row >= 1 && row <= a.rows() && col >= 1 && col <= a.cols()){
        r = row;
        c = col;
        return true;
    }

    return false;
}

  // Recommend a move for a player at (r,c):  A false return means the
  // recommendation is that the player should drop a poison pellet and not
  // move; otherwise, this function sets bestDir to the recommended
  // direction to move and returns true.
bool recommendMove(const Arena& a, int r, int c, int& bestDir)
{
  
  //save the original values before passing them into attemptMove
  int row = r; 
  int col = c;

  int x = check_rats(a, r, c); //current position
  int north = check_rats(a, r-1, c);
  int west = check_rats(a, r, c-1);
  int south = check_rats(a, r+1, c);
  int east = check_rats(a, r, c+1);

  int best = x; //by default remain where you are
  int count = 0; //used to keep track of whether or not best has changed

  if (attemptMove(a, NORTH, r, c) && north <= best){
      bestDir = NORTH;
      best = north;
      count++;
  }
  r = row; //undo the effects of calling attemptMove
  c = col;
  if (attemptMove(a, WEST, r, c) && west <= best){
      bestDir = WEST;
      best = west;
      count++;
  }
  r = row;
  c = col;
  if (attemptMove(a, SOUTH, r, c) && south <= best){
      bestDir = SOUTH;
      best = south;
      count++;
  }
  r = row;
  c = col;
  if (attemptMove(a, EAST, r, c) && east <= best){
      bestDir = EAST;
      best = east;
      count++;
  }
  r = row;
  c = col;

  if ( (best == x && a.getCellStatus(r, c) == EMPTY) || 
       (count == 0) )
      return false;

  return true;
}

int check_rats(const Arena& a, int r, int c){
    if (a.numberOfRatsAt(r, c) > 0) return 999; //there is a rat at this position so don't move there
    return a.numberOfRatsAt(r-1, c) + a.numberOfRatsAt(r+1, c) + 
           a.numberOfRatsAt(r, c-1) + a.numberOfRatsAt(r, c+1);
}

///////////////////////////////////////////////////////////////////////////
// main()
///////////////////////////////////////////////////////////////////////////

int main()
{
      // Create a game
      // Use this instead to create a mini-game:   Game g(3, 5, 2);
    Game g(10, 12, 40);

      // Play the game
    g.play();
}

///////////////////////////////////////////////////////////////////////////
//  clearScreen implementation
///////////////////////////////////////////////////////////////////////////

// DO NOT MODIFY OR REMOVE ANY CODE BETWEEN HERE AND THE END OF THE FILE!!!
// THE CODE IS SUITABLE FOR VISUAL C++, XCODE, AND g++ UNDER LINUX.

// Note to Xcode users:  clearScreen() will just write a newline instead
// of clearing the window if you launch your program from within Xcode.
// That's acceptable.  (The Xcode output window doesn't have the capability
// of being cleared.)

#ifdef _MSC_VER  //  Microsoft Visual C++

#pragma warning(disable : 4005)
#include <windows.h>

void clearScreen()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
    COORD upperLeft = { 0, 0 };
    DWORD dwCharsWritten;
    FillConsoleOutputCharacter(hConsole, TCHAR(' '), dwConSize, upperLeft,
                                                        &dwCharsWritten);
    SetConsoleCursorPosition(hConsole, upperLeft);
}

#else  // not Microsoft Visual C++, so assume UNIX interface

#include <iostream>
#include <cstring>
#include <cstdlib>

void clearScreen()  // will just write a newline in an Xcode output window
{
    static const char* term = getenv("TERM");
    if (term == nullptr  ||  strcmp(term, "dumb") == 0)
        cout << endl;
    else
    {
        static const char* ESC_SEQ = "\x1B[";  // ANSI Terminal esc seq:  ESC [
        cout << ESC_SEQ << "2J" << ESC_SEQ << "H" << flush;
    }
}

#endif