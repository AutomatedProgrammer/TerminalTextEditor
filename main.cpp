#include <iostream>
#include <string>
#include <unistd.h>
#include <termios.h>
#include <cctype>
#include <vector>
#include <sstream>
#include <sys/ioctl.h>
#include <fstream>
using namespace std;

#define CTRL_KEY(k) ((k) & 0x1f) //I have no idea how to rewrite this in C++

int cursorX = 0;
int cursorY = 0;
int screenrows = 0;
int screencols = 0;

void saveFile(const vector<char>&);

int getWindowSize(int *rows, int *cols) { //Pretty much copied from buildyourowntexteditor
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1;
    }
    else{
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
    
    
}


enum editorKey {
    BACKSPACE = 127,
    ARROW_LEFT = -24, //-24 Left
    ARROW_RIGHT = -23, //-23 right
    ARROW_UP = -22,   //-22 up
    ARROW_DOWN = -21, //-21 down
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN
};

struct termios orig_termios;
void disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        cerr << "Couldn't disable"; 
    //Sets the terminal attributes to the 
    //original terminal state
}

void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) cerr << "can't get attribute"; //This reads the current terminal attributes into a struct
    atexit(disableRawMode); //This calls disableRawMode when the program exits

    //c_lflag is for local flags, c_iflag is input flags, c_oflag is outputflags, c_cflag which are controlflags
    struct termios raw = orig_termios; //Make a copy of og terminals before making edits.
    raw.c_iflag &= ~(ICRNL |IXON); //IXON Disables ctrl-s and ctrl-q, ICRNL fices Ctrl-m
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); //Turns off ECHO, which is a state that prints each key to the terminal
    //Turns off ICANON which is canonical mode to read byte-by-byte instead of line-by-line
    //turns off IEXTEN which disables ctrl-v
    //ISIG i'm guessing is like icanon.
    raw.c_oflag &= ~(OPOST); //Turns off output processing so no "\r\n", now have to write out
    //the full "\r\n" to start a new line
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    //turning off stuff that is probably allready turned off
    raw.c_cflag |= (CS8);

    //raw.c_cc[VMIN] = 0;
    //raw.c_cc[VTIME] = 10;

    //Indexes into c_cc field which stands for control characters, an array of bytes that 
    //control various terminal settings
    //VMIN sets the minimum number of bytes of input needed before read() can return
    //Set to 0 so read returns as soon as input is read
    //VTIME sets the maximum amount of time to wait before read() returns. it is in 
    //tenths of a second, so its set to 10 to be one second.
    //if read() times out it returns 0 since thats the amount read.

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) cerr << "ERROR COULDN'T SET ATTRIBUTE"; //Sets the termianl to the raw struct
}

char editorReadKey() {
    int nread;
    char input = getchar();

    if (input == '\033') {
    getchar();
    switch(getchar())
    {
        case 65:
        return ARROW_UP;
        break;

        case 66:
        return ARROW_DOWN;
        break;

        case 67: 
        return ARROW_RIGHT;
        break;

        case 68:
        return ARROW_LEFT;
        break;

    }        
    }
    else {
        return input;
    }


}

void addBuffer(char input, vector<char>& buffer_vec)
{
    buffer_vec.push_back(input);
}

void removeBuffer(vector<char>& buffer_vec)
{
    buffer_vec.pop_back();
}

void editorProcessKeypress(vector<char>& buffer_vec) {
    char input = editorReadKey();

    switch (input) {
        case '\r':
        addBuffer('\r', buffer_vec);
        addBuffer('\n', buffer_vec);
        break;

        case CTRL_KEY('q'):
        cout.write("\x1b[2J", 4);
        cout.write("\x1b[H", 3);
        disableRawMode();
        exit(0);
        break;

        case CTRL_KEY('s'):
        saveFile(buffer_vec);
        break;

        case BACKSPACE:
        removeBuffer(buffer_vec);
        break;

        
        case ARROW_UP:
        if (cursorY != 0)
        {
            cursorY--;
        }
        break;

        case ARROW_DOWN:
        if (cursorY != screenrows - 1)
        {
            cursorY++;
        }
        break;

        case ARROW_LEFT:
        if (cursorX != 0)
        {
            cursorX--;
        }
        break;

        case ARROW_RIGHT:
        if (cursorX != screencols - 1)
        {
            cursorX++;
        }
        break;
        

        default:
        addBuffer(input, buffer_vec);
        break;
    }
}

void editorRefreshScreen(const vector<char>& buffer_vec) {

    cout.write("\x1b[?25l", 6);
    cout.write("\x1b[2J", 4);
    cout.write("\x1b[H", 3);
    cout.write("\x1b[?25h", 6); 
    
    for (int i = 0; i < buffer_vec.size(); i++) {
        cout << buffer_vec[i];
    }
    
}

void openFile(string filename, vector<char>& buffer_vec)
{
    ifstream ifs;
    char word;
    ifs.open(filename);
    while (ifs >> noskipws >> word)
    {
        buffer_vec.push_back(word);
    }
    ifs.close();
}

void saveFile(const vector<char>& buffer_vec)
{
    string filename;
    cout.write("\x1b[?25l", 6);
    cout.write("\x1b[2J", 4);
    cout.write("\x1b[H", 3);
    cout.write("\x1b[?25h", 6); 
    cout << endl;
    cout << "Save as...";
    cin >> filename;

    ofstream ofs;
    ofs.open(filename);

    for (int i = 0; i < buffer_vec.size(); i++)
    {
        if (buffer_vec[i] == '\r') ofs << '\r';
        else if (buffer_vec[i] == '\n') ofs << '\n';
        else
            ofs << buffer_vec[i];
    }
}

int main(int argc, char *argv[]) {
    vector<char> buffer;
    enableRawMode();
    getWindowSize(&screenrows, &screencols);
    
    if (argc >= 2)
    {
        openFile(argv[1], buffer);
    }
    
    while (true) {
        editorRefreshScreen(buffer);
        editorProcessKeypress(buffer);
    }
    return 0;
}

