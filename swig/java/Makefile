#
#       SWIG example make file for Linux
#
all:
	g++ -fPIC -c export.cpp -I../../include
	g++ -fPIC -c export_wrap.cpp
	g++ -shared export.o export_wrap.o -o libSimString.so

sample:
	javac simstring/*.java
	javac Sample.java

clean:
	rm -rf *.o *.so *.cpp *.class export.* simstring

purge:
	rm -rf *.o *.so *.cpp export.* simstring
