all: deflater pngCreater

deflater: 
	$(CC) -o deflater.exe src\deflater.c lib\libz.a

pngCreater: 
	$(CXX) -o pngCreater.exe src\pngCreater.cpp