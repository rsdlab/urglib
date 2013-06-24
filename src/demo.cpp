/**
 * @file demo.cpp
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <conio.h>
#else
#include <termios.h>
struct termios m_oldTermios;
#endif

#include "liburg.h"

// 特殊キーの定義
enum {
    UP = 65536,
    DOWN,
    LEFT,
    RIGHT,
    SPACE,
    ESCAPE,
};


int main(int argc, char* argv[]) {
  if (argc <= 2) {
    std::cout << "Invalid Usage." << std::endl;
    return -1;
  }
  std::cout << "URG DEMO" << std::endl;
  ssr::UrgBase urg(argv[1], atoi(argv[2]));

  urg.startMeasure();


  std::cout << "Waiting for Key Input" << std::endl;
  char c;
  std::cin >> c;
  return 0;
}

//// 画面用関数定義 ////


static void init_scr() {
#ifdef WIN32
	system("cls");
#else
  	struct termios myTermios;
	tcgetattr(fileno(stdin), &m_oldTermios);
	tcgetattr(fileno(stdin), &myTermios);
    
	myTermios.c_cc[VTIME] = 0;
#ifdef linux
	myTermios.c_cc[VMIN] = 0;
#else // MacOS
	myTermios.c_cc[VMIN] = 1;
#endif
	myTermios.c_lflag &= (~ECHO & ~ICANON);
	tcsetattr(fileno(stdin), TCSANOW, &myTermios);
#endif
}

static void clear_scr() {
#ifdef WIN32
  system("cls");
#else
  system("clear");
#endif
}

static void exit_scr() {
#ifdef WIN32
  system("cls");
#else
  system("reset");
#endif
}

static int myKbhit() {
#ifdef WIN32
  return _kbhit();
#else
  fd_set fdset;
  struct timeval timeout;
  FD_ZERO( &fdset ); 
  FD_SET( 0, &fdset );
  timeout.tv_sec = 0; 
  timeout.tv_usec = 0;
  int ret = select(0+1 , &fdset , NULL , NULL , &timeout );
  return ret;
#endif
}

static int myGetch() {
#ifdef WIN32
	char c = _getch();
	if(c == 0xFFFFFFE0) {
		c = _getch();
		if(c == 0x48) return UP;
		if(c == 0x50) return DOWN;
		if(c == 0x4B) return LEFT;
		if(c == 0x4D) return RIGHT;
	} else if(c == ' ') {
		 return SPACE;
	}

	return c;
#else
    int keys[5] = {-1, -1, -1, -1, -1};
    int key = getchar();
	switch(key) {
        case -1:
        case 0:
            return -1;
            
        case ' ':
            return SPACE;
        case 27:
            key = getchar();
            switch(key) {
                case -1:
                    return ESCAPE;
                case 79:
                    key = getchar();
                    return key;
                case '[':
                    for(int i = 0;i < 5;i++) {
                        if(key == -1 || key == 27) break;
                        keys[i] = key = getchar();
                    }
                    ungetc(key, stdin);
                    
                    switch(keys[0]) {
                        case 65: return UP;
                        case 66: return DOWN;
                        case 67: return RIGHT;
                        case 68: return LEFT;
                        default: return keys[0];
                    }
            }
	}
	return key;
#endif
}
