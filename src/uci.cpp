#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#endif

#include "uci.h"
#include "pvtable.h"
#include "movegen.h"
#include "makemove.h"
#include "time.h"

/*
 * Code taken from 
 * https://www.youtube.com/watch?v=EzkmJEkAmoY&index=68&list=PLZ1QII7yudbc-Ky058TEaOstZHVbT-2hg
 */

void Uci::loop(Board &board, SearchInfo &info) {
  setbuf(stdin, NULL);
  setbuf(stdout, NULL);

  char line[INPUTBUFFER];
  printf("id name %s\n", NAME);
  printf("id author Bart\n");
  printf("uciok\n");

  PvTable::init();

  while (true) {
    memset(&line[0], 0, sizeof(line));
    fflush(stdout);
    if (!fgets(line, INPUTBUFFER, stdin)) {
      continue;
    }

    if (line[0] == '\n') {
      continue;
    }

    if (!strncmp(line, "isready", 7)) {
      printf("readyok\n");
      continue;
    }
    else if (!strncmp(line, "position", 8)) {
      parse_position(line, board);
    }
    else if (!strncmp(line, "ucinewgame", 10)) {
      parse_position((char*) "position startpos\n", board);
    }
    else if (!strncmp(line, "go", 2)) {
      parse_go(line, info, board);
    }
    else if (!strncmp(line, "quit", 4)) {
      info.quit = true;
      break;
    }
    else if (!strncmp(line, "uci", 3)) {
      printf("id name %s\n", NAME);
      printf("id author Bart\n");
      printf("uciok\n");
    }
    if (info.quit) break;
  } 

  PvTable::free_table();
}

void Uci::parse_position(char *lineIn, Board &board) {
  lineIn += 9;
  char *ptrChar = lineIn;

  if (strncmp(lineIn, "startpos", 8) == 0) {
    board.init();
  }
  else {
    ptrChar = strstr(lineIn, "fen");
    if (ptrChar == NULL) {
      board.init();
    }
    else {
      ptrChar += 4;
      board.parse_fen(ptrChar);
    }
  }

  ptrChar = strstr(lineIn, "moves");
  int move;
  if (ptrChar != NULL) {
    ptrChar += 6;
    while (*ptrChar) {
      move = MoveGenerator::parse_move(ptrChar, board);
      if (move == NOMOVE) break;
      MoveMaker::make_move(board, move);
      board.ply = 0;
      while (*ptrChar && *ptrChar != ' ') ptrChar++;
      ptrChar++;
    }
  }

  board.print_board();
}

void Uci::parse_go(char *line, SearchInfo &info, Board &board) {
  int depth = -1, movestogo = 30, movetime = -1;
  int time = -1, inc = 0;
  char *ptr = NULL;
  info.time_set = false;

  if ((ptr = strstr(line, "infinite"))) {
    ;
  }

  if ((ptr = strstr(line, "binc")) && board.side == Black) {
    inc = atoi(ptr + 5);
  }

  if ((ptr = strstr(line, "winc")) && board.side == White) {
    inc = atoi(ptr + 5);
  }

  if ((ptr = strstr(line, "wtime")) && board.side == White) {
    time = atoi(ptr + 6);
  }

  if ((ptr = strstr(line, "btime")) && board.side == Black) {
    time = atoi(ptr + 6);
  }

  if ((ptr = strstr(line, "movestogo"))) {
    movestogo = atoi(ptr + 10);
  }

  if ((ptr = strstr(line, "movetime"))) {
    movetime = atoi(ptr + 9);
  }

  if ((ptr = strstr(line, "depth"))) {
    depth = atoi(ptr + 6);
  }

  if (movetime != -1) {
    time = movetime;
    movestogo = 1;
  }

  info.start_time = Time::get_current_time();
  info.depth = depth;

  if (time != -1) {
    info.time_set = true;
    time /= 50; //movestogo;
    time -= 50;
    info.stop_time = info.start_time + time + inc;
  }

  if (depth == -1) {
    info.depth = MAXDEPTH;
  }

  printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
      time, info.start_time, info.stop_time, info.depth, info.time_set);

  Searcher::search_position(board, info);  
}

void Uci::read_input(SearchInfo &info) {
  int bytes;
  char input[256] = "", *endc;
  if (input_waiting()) {    
  	info.stopped = true;
		do {
		  bytes=read(fileno(stdin),input,256);
		} while (bytes < 0);
		endc = strchr(input,'\n');
		if (endc) *endc = 0;

		if (strlen(input) > 0) {
			if (!strncmp(input, "quit", 4))    {
			  info.quit = true;
			}
		}
		return;
  }
}

// http://home.arcor.de/dreamlike/chess/
bool Uci::input_waiting()
{
#ifndef WIN32
  fd_set readfds;
  struct timeval tv;
  FD_ZERO (&readfds);
  FD_SET (fileno(stdin), &readfds);
  tv.tv_sec=0; tv.tv_usec=0;
  select(16, &readfds, 0, 0, &tv);

  return (FD_ISSET(fileno(stdin), &readfds));
#else
  static int init = 0, pipe;
  static HANDLE inh;
  DWORD dw;

  if (!init) {
    init = 1;
    inh = GetStdHandle(STD_INPUT_HANDLE);
    pipe = !GetConsoleMode(inh, &dw);
    if (!pipe) {
      SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
      FlushConsoleInputBuffer(inh);
    }
  }
  if (pipe) {
    if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
    return dw;
  } 
  else {
    GetNumberOfConsoleInputEvents(inh, &dw);
    return dw <= 1 ? 0 : dw;
	}
#endif
}

