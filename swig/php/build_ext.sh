g++ `php-config --includes` -fPIC -c export_wrap.cpp export.cpp
g++ -shared export.o export_wrap.o -o simstring.so
