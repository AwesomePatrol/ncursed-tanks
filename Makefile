all: client server

DOXYGENCONF=doxygen.conf

CFLAGS += -std=c99

OBJS_server := server.o common.o debug.o map_gen.o
OBJS_client := client.o common.o debug.o cl_proto.o game.o map_gen.o render.o draw.o

server: $(OBJS_server) $(COBJS_server)
	gcc -o $@ -pthread -lm -L/usr/lib $(OBJS_server) $(COBJS_server) 

client: $(OBJS_client) $(COBJS_client)
	gcc -o $@ -lm -lncurses -L/usr/lib $(OBJS_client) $(COBJS_client)

-include *.d

%.o: %.c
	gcc $(CFLAGS) -c -g3 -o $@ $<
	gcc -MM $< > $*.d

clean:
	rm -rf $(OBJS) $(OBJS_server) $(OBJS_client)
	rm -rf *.d
	rm -rf server client

test: all

doc:
	doxygen $(DOXYGENCONF)
