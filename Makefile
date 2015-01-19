all: server client config_menu

DOXYGENCONF=doxygen.conf

-include localdefs.mk

ifdef DEBUG
	CFLAGS += -O1 -std=c99 -D _DEBUG -Wall -Wno-switch
else
	CFLAGS += -O2 -std=c99
endif

OBJS_server := server.o common.o server_game.o server_abilities.o server_data.o server_net.o dyn_arr.o server_updates_queue.o config.o abilities_config.o file_io.o debug.o map.o shot.o
OBJS_client := client.o common.o debug.o dyn_arr.o map.o shot.o config.o file_io.o client_net.o client_game.o client_scene.o client_render.o client_draw.o
OBJS_config_menu := config_menu.o debug.o config.o file_io.o

server: $(OBJS_server) $(COBJS_server)
	gcc -o $@ -pthread -lm -L/usr/lib $(OBJS_server) $(COBJS_server) 

client: $(OBJS_client) $(COBJS_client)
	gcc -o $@ -lm -lncurses -L/usr/lib $(OBJS_client) $(COBJS_client)

config_menu: $(OBJS_config_menu) $(COBJS_config_menu)
	gcc -o $@ -lm -lmenu -lncurses -L/usr/lib $(OBJS_config_menu) $(COBJS_config_menu)

-include *.d

%.o: %.c
	gcc $(CFLAGS) -c -g3 -o $@ $<
	gcc -MM $< > $*.d

clean:
	rm -rf $(OBJS_server) $(OBJS_client) $(OBJS_config_menu)
	rm -rf *.d
	rm -rf server client config_menu

test: all

doc:
	doxygen $(DOXYGENCONF)
