/*** INCLUDES ***/
#include <iostream>
#include <unistd.h>
#include <termio.h>
#include <ctype.h>

/*** DEFINES ***/

//ANDs a character with 00011111, in binary. Mirrors what ctrl does in terminal, where it strips 5-6 bits when pressed with ctrl.
#define CTRL_KEY(k) ((k) & 0x1f)


/*** DATA ***/

struct termios origTermios;

/*** TERMINAL ***/

void die(const char* s) { //error handling
    perror(s);
    //perror looks at errno and prints the descriptive message for it.
    exit(1);
}

void disableRawMode() {
    //we'll save a copy of termios in orig. state, then reapply it with tcsetattr().
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &origTermios) == -1) {
        die("tcsetattr");
    }
}

void enableRawMode() {
    //The same mode when you have to type a password in the terminal.
    if(tcgetattr(STDIN_FILENO, &origTermios) == -1) { //gets the current attributes of the terminal
        die("tcgetattr"); 
    }
    atexit(disableRawMode);  //exits raw mode when leaving.


    struct termios raw = origTermios;

    raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP | ICRNL | IXON);
    //this disables ctrl-s and ctrl-q. IXON unlike the three below IS actually an input, so we use iflag.
    //&= means bitwise AND operator, kinda like +=, must use ~ negation unlike with |= which is OR bitwise.
    //ICRNL is for ctrl-m and fixes ctrl-j. cr is for carriage return, nl for new line.
    //brkint, inpck, istrp likely turned off on default but this is just in case. TRADITION


    raw.c_oflag &= ~(OPOST);
    //output. Turned off \n to \r\n translation.
    //to fix what we did here by the way, we'll need to add it to the printf statement.

    raw.c_cflag |= (CS8);
    //legacy tradition as well.
    //this is not a flag, it just sets char size to 8 bits per byte, typically this is default but we make sure its on with OR.

    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); //Gets echo attribute
    //c_lflag is for "local" flags, akin to a misc. flag. "A dumping ground for other state."
    //c_iflag is for input, c_oflag is for output. c_cflag is for control flags. Ergo, it's basically all of them.
    //ICANON let's us to turn off canonical mode, allowing for actual byte-to-byte readings instead of line-to-line.
    //ISIG here allows me to disable SIGINT and SIGTSTP, ctrl-c and ctrl-z respectively.
    //IEXTEN disables ctrl-v.

    

    //inactive thingy
    //vmin and vtime are indexes into c_cc(control chars), vmin sets min num of bytes read before read() can return.
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1; //vtime sets max time to wait before read() returns. It's set to 1/10 seconds.
    //exp: vmin returns as soon as there is input to be read, vtime sets time to wait before doing so.

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die ("tcsetattr");
    
}

char editorReadkey() {
    //this function waits for a keypress to be sent and to return it. Deals with low-level terminal input.
    int nread;
    char c;

    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) die("read");
    }
    return c;
}

/*** INPUT ***/

void editorProcessKeyPresses() {
    //waits for a keypress and then handles it. Basically mapping keys!
    char c = editorReadkey();

    switch (c) {
        case CTRL_KEY('q'):
            exit(0);
            break;
    }
}

/*** INIT ***/

int main() {

    enableRawMode();

    while (1) {
        editorProcessKeyPresses();
    }
    
    
    return 0;
}