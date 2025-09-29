CFLAGS = -Wall -Werror -O3
LDFLAGS = -lGL -lglfw


.PHONY: all run renderdoc

all: main run

main: main.c
	cc $(CFLAGS) -o main main.c $(LDFLAGS)

run: main
	./main

renderdoc: main
	WAYLAND_DISPLAY= XDG_SESSION_TYPE=x11 qrenderdoc renderdoc.cap
