CFLAGS = -Wall -Werror -O0
LDFLAGS = -lGL -lglfw -lm


.PHONY: all run renderdoc

all: cube run

cube: cube.c math.c shader.c file.c cube.vert cube.frag
	cc $(CFLAGS) -o cube cube.c $(LDFLAGS)

run: cube
	./cube

renderdoc: cube
	WAYLAND_DISPLAY= XDG_SESSION_TYPE=x11 qrenderdoc renderdoc.cap
