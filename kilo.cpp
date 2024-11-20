#include <iostream>
#include <unistd.h>
#include <termio.h>


struct termios origTermios;

void disableRawMode() {
    //we'll save a copy of termios in orig. state, then reapply it with tcsetattr().
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &origTermios);
}


void enableRawMode() {
    //The same mode when you have to type a password in the terminal.
    

    tcgetattr(STDIN_FILENO, &origTermios); //gets the current attributes of the terminal
    atexit(disableRawMode);//exits raw mode when leaving.

    struct termios raw = origTermios;

    raw.c_lflag &= ~(ECHO | ICANON); //Gets echo attribute
    //c_lflag is for "local" flags, akin to a misc. flag. "A dumping ground for other state."
    //c_iflag is for input, c_oflag is for output. c_cflag is for control flags. Ergo, it's basically all of them.
    //ICANON let's us to turn off canonical mode, allowing for actual byte-to-byte readings instead of line-to-line.

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);//applies the new setting to terminal, the second parameter tells where it'll be done.
    //ergo, it pends output to be written and discards any unread input.
}


int main() {

    enableRawMode();

    char c;

    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q');
    /*
    This is from the unistd library, it is to read input at each byte into c, untill there are no more, then return the number of
    bytes.
    To use q to quit, we make sure c != q!*/
    
    return 0;
}