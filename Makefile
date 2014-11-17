all: game server

DOXYGENCONF=doxygen.conf

OBJS_server := server.o parser.o

server: $(OBJS_server) $(COBJS_server)
	gcc -o $@ -lm -L/usr/lib -lz $(OBJS_server) $(COBJS_server) 

game: $(OBJS) $(COBJS)
	gcc -o $@ -lm -L/usr/lib -lncurses -lz $(OBJS) $(COBJS) 

-include *.d

%.o: %.c
	gcc -c -g3 -o $@ $<
	gcc -MM $< > $*.d

clean:
	rm -rf $(OBJS) $(OBJS_server)
	rm -rf $(COBJS) $(COBJS_server)
	rm -rf server game menu

test: all

doc:
	doxygen $(DOXYGENCONF)
