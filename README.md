(for now, works only in windows)<br>
Create simple 8 bit rgb sample depth valued PNG files from rgb pixel data files.

example usage;
<br>
<br>
<br>
sampleImageDataFile:

0x10 0x15 0x25 0x10 0x15 0x25 0x10 0x15 0x25

0x15 0x45 0x50 0x15 0x45 0x50 0x15 0x45 0x50

0x20 0x30 0x40 0x20 0x30 0x40 0x20 0x30 0x40

<br>
<br>
in shell:
	pngCreater.exe sampleImageDataFile 3 3 myImage.png
<br>
<br>
-----------
<br>
<br>
to compile:

	just compile using the makefile
<br>
when compiling with mingw-w64's mingw32-make;<br>
add this at the beginning of the makefile: 
	
	CC=gcc
<br>
-----------
<br>
<br>
usage: 

	start pngCreater.exe in shell with these parameters: 
	pngCreater.exe <image data file's path> <image width> <image height> <output file's path>
<br>
<br>
pngCreater and deflater are portable, have no dependecies other than each other.
