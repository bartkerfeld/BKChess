BKChess is a simple chess engine developed as a project for CSCI 4511w

It has only been tested on Ubuntu 16.04 using g++ 5.3.1. I cannot imagine there
should be any issues running it on other linux versions but your millage may vary.

To compile type "make clean" followed by "make release"

Tested on Areana 3.5.1 GUI using Wine in Ubuntu
cd .wine/drive_c/Program Files (x86)/Arena/Engines/
cp <path_to_bkchess>/bkchess .
mv bkchess bkchess.exe

Then in Areana, select Engines > Install New Engine and pick bkchess using UCI protocol

ACKNOWLEDGEMENTS:
  - A lot of the bitboard ideas and code came from chessprogramming.wikispace.com
  - Uci, search, evaluation where heavily based on Vice engine covered by YouTube channel Bluefever
