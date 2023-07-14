#include <ncurses.h>
#include <list>
#include <fstream>

using namespace std;

#define CTRL_KEY(k) ((k) & 0x1f) //Control key

int main(int argc, char** argv)
{
    int x = 0, y = 0;
    int lc = 0;
    list<int> buffer;
    list<int>::iterator it = buffer.begin(); //Viewer iterator
    list<int>::iterator e_it = buffer.end(); //Editor iterator
    int ch;
    char cch;
    ifstream ifs;
    ofstream ofs;
    //ifs.open(argv[1]);
    ifs.open("Test.txt");
    while (ifs >> noskipws >> cch)
    {
        ch = static_cast<int>(cch);
        buffer.push_back(ch);
    }
    ifs.close();

    initscr(); //Start curses
    raw(); //raw mode turned on
    keypad(stdscr, true); //Enable f1, f2
    noecho(); //No echo while using getch
    
    for (it = buffer.begin(); it != buffer.end(); it++)
    {
        ch = *it;
        printw("%c", ch);
        x++;
    }
    //getsyx(y, x);
    while(true)
    {
        //getsyx(y,x); 
        ch = getch();
        //if (ch == 17) break;
        if (ch == CTRL_KEY('q'))
        {
            clear();
            printw("Quit program? Press CTRL+Q to confirm. Any key to go back.");
            ch = getch();
            if (ch == CTRL_KEY('q')) break;
            else {
                clear();
                for (it = buffer.begin(); it != buffer.end(); it++)
                {
                    ch = *it;
                    printw("%c", ch);
                } 
                continue;
            }
        }
        else if (ch == CTRL_KEY('s'))
        {
            //ofs.open(argv[1]);
            ofs.open("Test.txt");
            for (it = buffer.begin(); it != buffer.end(); it++)
            {
                cch = static_cast<char>(*it);
                if (cch == '\r') ofs << '\r';
                else if (cch == '\n') ofs << '\n';
                else
                    ofs << cch;
            }
            ofs.close();
            continue;
        }
        else if (ch == 263)
        {
            //swap(*e_it, buffer.back());
            
            if (e_it != buffer.end()) e_it = buffer.erase(e_it);
            else if (e_it != buffer.begin())
            {
                buffer.pop_back();
            }
            x = 0;
            clear();
            for (it = buffer.begin(); it != buffer.end(); it++)
            {
                ch = *it;
                printw("%c", ch);
                x++;
            }
            
            //move(y,x);
            
            continue;
        }
        else if (ch == KEY_UP)
        {
            //getsyx(y, x);
            //y--;
            //move(y, x);
            continue;
        }
        else if (ch == KEY_DOWN)
        {
            //getsyx(y, x);
            //y++;
            //move(y, x);
            continue;
        }
        else if (ch == KEY_RIGHT)
        {
            if (e_it != buffer.end())
            {
                //getsyx(y, x);
                x++;
                e_it++;
                move(y, x);
            }
            continue;
        }
        else if (ch == KEY_LEFT)
        {
            if (e_it != buffer.begin() && x-1 > 0)
            {
                //getsyx(y, x);
                x--;
                e_it--;
                move(y, x);
            }
            continue;
        }
        clear();
        
        buffer.insert(e_it, ch);
        x++;
        for (it = buffer.begin(); it != buffer.end(); it++)
        {
            ch = *it;
            printw("%c", ch);
        }
        move(y,x);
        
    }
    
    refresh();
    endwin();
    return 0;
}
//TODO
//Let the user move the cursor around the lines.
//That's it.
