


del /q ..\main.dll
g++ -shared -o ../main.dll main.cpp -static -lsqlite3
@rem g++ -pthread -shared -o main.dll main.cpp -I ./include  liblua54.a -g
@rem g++ -shared -o main.dll main.cpp -I ./include  liblua54.a -g
@rem g++ -o a.exe main.cpp -lsqlite3
rem ssp /g Ghost_Mine
rem
