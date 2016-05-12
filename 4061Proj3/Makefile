all: mm.c packet_receiver.c packet_sender.c
	gcc -o mm.o -c mm.c
	gcc -o packet_receiver packet_receiver.c mm.o
	gcc -o packet_sender packet_sender.c mm.o
	gcc -o main_mm main_mm.c mm.o
	gcc -o main_malloc main_malloc.c mm.o
clean: 
	rm mm.o packet_sender packet_receiver main_malloc main_mm
