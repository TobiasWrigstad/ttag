all:
	astyle --indent=spaces=2 --style=gnu tt.c
	gcc -Wall -pedantic -O3 -o tt tt.c
