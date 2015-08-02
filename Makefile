
all:
	cd src && $(MAKE)

clean:
	cd src && $(MAKE) clean
	rm -f *.debug

doc:
	cd src && $(MAKE) doc
