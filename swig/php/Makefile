#
#	   SWIG example make file for Linux
#
all:
	g++ `php-config --includes` -fPIC -c export.cpp -I../../include
	g++ `php-config --includes` -fPIC -c export_wrap.cpp
	g++ -shared export.o export_wrap.o -o simstring.so

clean:
	rm -rf *.o *.so *.h *.cpp *.php export.*

purge:
	rm -rf *.o *.so *.h *.cpp export.*
