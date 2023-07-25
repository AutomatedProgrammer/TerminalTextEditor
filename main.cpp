#include <ncurses.h> //Ncurses library so my life is easier handling the terminal
#include <list> //Need for the random insert and removal
#include <fstream> //File opening and saving

using namespace std;

#define CTRL_KEY(k) ((k) & 0x1f) //Control key macro.

//A copy of the std::find, except it has extra parameters to change the position of the cursor and the reverse_iterator.
//Basically matches the cursor pos with the iterator pos
list<int>::iterator my_find(list<int>::iterator first, list<int>::iterator last, list<int>::reverse_iterator& re_it, const int& val, int& x_pos)
{
    while (first != last) 
    {
        if (*first == val) return first;
        --re_it;
        ++first; 
        ++x_pos;
    }
    return last;
}

//std::find with a few extra features. 
//It searches through the list backwards, skipping the first newline it finds till it finds the second one.
//The reason it skips over the first newline is because the first newline would lead the cursor to the back of the current line.
/* example
First line.
Here is another line. <- Cursor is here
After searching it'll put the cursor here.
First line.
->Here is a line. Since that's where the first new line is.
But if I have it skip over the first newline it'll go here.
->First line
Here is another line.
*/
//So after it gets to the second newline, it'll move the iterator and cursor forward by however many characters it had to go over to get to the back
//Thus the char_counter
/* example
->First line (10 chars in the first line counted I think) So it moves the cursor by 10 to the front of the line, at the newline for the second line.
Here is another line.
*/
list<int>::reverse_iterator my_rfind(list<int>::reverse_iterator first, list<int>::reverse_iterator last, const int& val, int& x_pos)
{
    bool first_newline = false;
    int char_counter = 0;
    while (first != last) 
    {
        if (*first == val) {
            if (first_newline == true)
            {
                advance(first, -char_counter);
                return first;
            }
            else
            {
                first_newline = true;
                ++first;
            }
        }
        if (first_newline == false)
        {
            ++first; 
        }
        else if (first_newline == true)
        {
            first++;
            char_counter++;
            x_pos++;
        }
    }
    return last;
}

int main(int argc, char** argv) //I have these arguments for main since I intended this program to be executed like ./ncurses_editor (filename)
{
    int x = 0, y = 0, max_y = 0, cur_y = 0, cur_x = 0;
    int x_offset = 0; //Determines the offset of x from the newline.
    list<int> buffer;
    list<int>::iterator it = buffer.begin(); //Viewer iterator for viewing the current buffer.
    list<int>::iterator e_it = buffer.end(); //Editor iterator for editing the current buffer.
    list<int>::reverse_iterator re_it = buffer.rbegin(); //Reverse iterator for the sake of being able to move the cursor up
    int ch; 
    char cch;

    //The reason I have an int list rather than a char list is because I used getch() which returns the int for a char.

    ifstream ifs;
    ofstream ofs;
    ifs.open(argv[1]);
    while (ifs >> noskipws >> cch) //Loop to put all the chars into int form into the buffer.
    {
        ch = static_cast<int>(cch);
        buffer.push_back(ch);
    }
    ifs.close();

    initscr(); //Start curses
    raw(); //raw mode turned on
    keypad(stdscr, true); //Enable f1, f2
    noecho(); //No echo while using getch
    
    for (it = buffer.begin(); it != buffer.end(); it++) //For loop here prints the contents of the current file.
    {
        ch = *it;
        printw("%c", ch);
        x++;
        if (ch == 10)
        {
            y++;
            max_y++;
            x = 0;
        }
        
    }
    //Logic loop
    while(true)
    {
        ch = getch();
        if (ch == CTRL_KEY('q'))
        {
            clear();
            printw("Quit program? Press CTRL+Q to confirm. Any key to go back.");
            ch = getch();
            if (ch == CTRL_KEY('q')) break;
            else {
                max_y = 0;
                x = 0;
                y = 0;
                clear();
                for (it = buffer.begin(); it != buffer.end(); it++)
                {
                    ch = *it;
                    printw("%c", ch);
                    x++;
                    if (ch == 10)
                    {
                        y++;
                        max_y++;
                        x = 0;
                    }
                }
                x -= x_offset;
                move(y,x);
                continue;
            }
        }
        else if (ch == CTRL_KEY('s'))
        {
            ofs.open(argv[1]);
            for (it = buffer.begin(); it != buffer.end(); it++)
            {
                cch = static_cast<char>(*it); //Cast to convert the ints into chars to write into file
                if (cch == '\r') ofs << '\r';
                else if (cch == '\n') ofs << '\n';
                else
                    ofs << cch;
            }
            ofs.close();
            continue;
        }
        else if (ch == 263) //This is the backspace
        {
            if (e_it != buffer.begin() && e_it != buffer.end()) //This if statements checks to see if the the editor_iterator is at the beginning. If the buffer was erased the editor_iterator would point to nothing and crash the program.
            {
                if (*e_it == 10) //If the editor_iterator points to a newline we adjust the cursor position.
                {
                    y--;
                    max_y--;
                }
                buffer.erase(e_it--);
            }
            else if (e_it == buffer.end() && e_it != buffer.begin()) //If it's at the end theres no need to do any erasing from random positions so we just pop it back.
            {
                if (*e_it == 10)
                {
                    y--;
                    max_y--;
                }
                buffer.pop_back();
            }

            cur_y = y; //We save the current position of the cursor to make sure you can't have the cursor go off screen.
            cur_x = x;
            x = 0;
            y = 0;
            max_y = 0;
            
            clear();
            for (it = buffer.begin(); it != buffer.end(); it++) //Redraw the entire buffer after erasing a character. Yes I know it'd be more efficient to just erase a single line and redraw that but I have no idea how to do that lol.
            {
                ch = *it; 
                if (ch == 10 && y < cur_y) //These two if statements just make sure the cursor is in the right position (EoL I think)
                {
                    y++;
                    x = 0; 
                }
                max_y++;
                if (x < cur_x){
                    x++;
                }
                printw("%c", ch);        
            }
            x--;
            move(y,x);
            
            continue;
        }
        else if (ch == KEY_UP)
        {
            if (y > 0)
            {
                
                x = 0;
                x_offset = 0;
                
                re_it = my_rfind(re_it, buffer.rend(), 10, x); //Uses the reverse iterator to search backwards for the next second newline.
                e_it = re_it.base(); //Sets the edtior_iterator to that position.
                
                while (*e_it != 10 && e_it != buffer.end()) //If it's not at the newline then it moves the iterator forward till it is. It also keeps the reverse_iterator position inline with the editor_iterator.
                {
                    re_it--;
                    e_it++;
                }
                
                y--;
                move(y, x);
            }
            continue;
        }
        else if (ch == KEY_DOWN)
        {
            if (y < max_y && e_it != buffer.end()) //Checks to see if the cursor position at the last line or the editor_iterator is at the end so the cursor doesn't go off the lines.
            {
               
                x_offset = 0;
                x = 0;
                e_it = my_find(e_it, buffer.end(), re_it, 10, x);
                
                e_it++;
                re_it--;
                while (*e_it != 10 && e_it != buffer.end()) //Same as the while loop on the key up. Except it also moves the cursor position to the end of line.
                {
                    e_it++;
                    re_it--;
                    x++;
                }
                
                y++;
                move(y, x);
            }
            continue;
        }
        else if (ch == KEY_RIGHT)
        {
            if (e_it != buffer.end() && *e_it != 10) //If not at the end of the line or buffer moves the cursor forward. Reduces the x_offset
            {
                x_offset--;
                x++;
                e_it++;
                move(y, x);
            }
            continue;
        }
        else if (ch == KEY_LEFT)
        {
            if (e_it != buffer.begin() && x - 1 > 0) //If not at the beginning of the line or buffer moves the cursor backwards.
            {
                x_offset++;
                x--;
                e_it--;
                move(y, x);
            }
            continue;
        }
        clear(); //Clears the screen before redrawing.
        //Inserts the inputted character provided it's not one of the special characters (backspace, down arrow, up arrow, ctrl-s/ctrl-q)
        buffer.insert(e_it, ch);
        x++; //Moves cursor forward
        if (ch == 10) //If the user pressed enter it moves the cursor down by 1
        {
            y++;
            max_y++;
            x = 0;
        }
        for (it = buffer.begin(); it != buffer.end(); it++) //Prints contents of the buffer.
        {
            ch = *it;
            printw("%c", ch);
        }
        
        move(y,x); //Repositions the cursor
        
        
    }
    
    refresh();
    endwin();
    return 0;
}