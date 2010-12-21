#
#       SWIG example make file for Linux
#
all:
	g++ -fPIC -c export.cpp -I../../include
	g++ -fPIC -c export_wrap.cxx
	g++ -shared export.o export_wrap.o -o libSimString.so

clean:
	rm -rf *.o *.cxx *.class export.* simstring

purge:
	rm -rf *.o *.so *.cxx export.* simstring
