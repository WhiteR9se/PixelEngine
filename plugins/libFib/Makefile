.PHONY: all clean

all:
	@echo "|- Building lib Fib"
	mkdir -p ../../bin/debug/plugins/
	gcc -shared -fPIC -O3 fibonnaci.c -o ../../bin/debug/plugins/libFib.so

clean:
	rm ../../bin/debug/plugins/libFib.so	
