#include <ncurses.h>
#include <list>
#include <algorithm>
#include <fstream>

using namespace std;

#define CTRL_KEY(k) ((k) & 0x1f) //Control key

list<int>::iterator my_find(list<int>::iterator first, list<int>::iterator last, list<int>::reverse_iterator& re_it, const int& val, int& x_pos)
{
    auto first_backup = first;
    auto x_pos_backup = x_pos;
    while (first != last) 
    {
        if (*first == val) return first;
        --re_it;
        ++first;
        ++x_pos;
    }
    //x_pos = x_pos_backup;
    return last;
    //return last;
}

list<int>::reverse_iterator my_rfind(list<int>::reverse_iterator first, list<int>::reverse_iterator last, const int& val, int& x_pos)
{
    auto first_backup = first;
    auto x_pos_backup = x_pos;
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
                //--x_pos;
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
        //++x_pos;
    }
    if (first == last)
    {
        return last;
    }
    //x_pos = x_pos_backup;
    //return last;
    //return last;
}

int main(int argc, char** argv)
{
    int x = 0, y = 0, max_y = 0;
    int x_offset = 0;
    list<int> buffer;
    list<int>::iterator it = buffer.begin(); //Viewer iterator
    list<int>::iterator e_it = buffer.end(); //Editor iterator
    list<int>::reverse_iterator re_it = buffer.rbegin(); //Reverse iterator for the sake of being able to move the cursor up
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
        if (ch == 10)
        {
            y++;
            max_y++;
            x = 0;
        }
        
    }
    //getsyx(y, x);
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
            if (e_it != buffer.begin() && e_it != buffer.end()) 
            {
                if (*e_it == 10)
                {
                    y--;
                    max_y--;
                }
                buffer.erase(e_it--);
            }
            else if (e_it == buffer.end() && e_it != buffer.begin()) 
            {
                if (*e_it == 10)
                {
                    y--;
                    max_y--;
                }
                buffer.pop_back();
            }

            x = 0;
            y = 0;
            max_y = 0;
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
        else if (ch == KEY_UP)
        {
            if (y > 0)
            {
                x = 0;
                //Reverse iterator code
                
                re_it = my_rfind(re_it, buffer.rend(), 10, x);
                e_it = re_it.base();

                //e_it = my_rfind(e_it, buffer.begin(), 10, x);
                //e_it++;
                while (*e_it != 10 && e_it != buffer.end())
                {
                    break;
                    e_it++;
                    
                }
                
                y--;
                move(y, x);
            }
            continue;
        }
        else if (ch == KEY_DOWN)
        {
            if (y < max_y)
            {
                x = 0;
                e_it = my_find(e_it, buffer.end(), re_it, 10, x);
                
                e_it++;
                re_it--;
                while (*e_it != 10 && e_it != buffer.end())
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
            if (e_it != buffer.end() && *e_it != 10)
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
            if (e_it != buffer.begin() && x - 1 > 0)
            {
                x_offset++;
                x--;
                e_it--;
                move(y, x);
            }
            continue;
        }
        clear();
        
        buffer.insert(e_it, ch);
        x++;
        if (ch == 10)
        {
            y++;
            max_y++;
            x = 0;
        }
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
//1. Add in the second dimension.