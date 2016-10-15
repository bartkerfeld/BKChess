all:
	g++ main.cpp bitboard.cpp board.cpp movegen.cpp makemove.cpp perft.cpp zobrist.cpp search.cpp time.cpp pvtable.cpp evaluate.cpp uci.cpp -o bkchess

release:
	g++ -DNDEBUG -O2 main.cpp bitboard.cpp board.cpp movegen.cpp makemove.cpp perft.cpp zobrist.cpp search.cpp time.cpp  pvtable.cpp evaluate.cpp uci.cpp -o bkchess

clean:
	rm -f bkchess
