#include <iostream>
#include <fstream>
#include "zlib.h"
#include <windows.h> //for starting up the deflator.exe
using namespace std;

//-----CRC functions START
// to calculate crc -> unsigned long crc(unsigned char *buf, int len)

   /* Table of CRCs of all 8-bit messages. */
   unsigned long crc_table[256];
   
   /* Flag: has the table been computed? Initially false. */
   int crc_table_computed = 0;
   
   /* Make the table for a fast CRC. */
   void make_crc_table(void)
   {
     unsigned long c;
     int n, k;
   
     for (n = 0; n < 256; n++) {
       c = (unsigned long) n;
       for (k = 0; k < 8; k++) {
         if (c & 1)
           c = 0xedb88320L ^ (c >> 1);
         else
           c = c >> 1;
       }
       crc_table[n] = c;
     }
     crc_table_computed = 1;
   }
   
   /* Update a running CRC with the bytes buf[0..len-1]--the CRC
      should be initialized to all 1's, and the transmitted value
      is the 1's complement of the final running CRC (see the
      crc() routine below)). */
   
   unsigned long update_crc(unsigned long crc, unsigned char *buf,
                            int len)
   {
     unsigned long c = crc;
     int n;
   
     if (!crc_table_computed)
       make_crc_table();
     for (n = 0; n < len; n++) {
       c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
     }
     return c;
   }
   
   /* Return the CRC of the bytes buf[0..len-1]. */
   unsigned long crc(unsigned char *buf, int len)
   {
     return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
   }

   //-----CRC functions END


//unsigned int to 4 Byte char array (hexadecimal)
void intToBytes(unsigned int n,unsigned char* ptr){
    ptr[0] = (n >> 24) & 0xFF;
    ptr[1] = (n >> 16) & 0xFF;
    ptr[2] = (n >> 8) & 0xFF;
    ptr[3] = n & 0xFF;
}


// Class for PNG Chunks

/* Reminder

Chunk layout:
Each chunk consists of four parts:
Length
A 4-byte unsigned integer giving the number of bytes in the chunk's data field. The length counts only the data field, not itself, the chunk type code, or the CRC. Zero is a valid length. Although encoders and decoders should treat the length as unsigned, its value must not exceed (2^31)-1 bytes.
Chunk Type
A 4-byte chunk type code. For convenience in description and in examining PNG files, type codes are restricted to consist of uppercase and lowercase ASCII letters (A-Z and a-z, or 65-90 and 97-122 decimal). However, encoders and decoders must treat the codes as fixed binary values, not character strings. For example, it would not be correct to represent the type code IDAT by the EBCDIC equivalents of those letters. Additional naming conventions for chunk types are discussed in the next section.
Chunk Data
The data bytes appropriate to the chunk type, if any. This field can be of zero length.
CRC
A 4-byte CRC (Cyclic Redundancy Check) calculated on the preceding bytes in the chunk, including the chunk type code and chunk data fields, but not including the length field. The CRC is always present, even for chunks containing no data. See CRC algorithm.
*/

/* use the constructor to create a PNG chunk with wanted parameters. Afterwards fill the chunks data in binary format as you wish.
   --for IDAT chunks, fill the data as you wish (don't forget to include interlace method at the beginning of every scanline)
   then use Write(ofstream &stream) method to write the chunk to a file via given ofstream.
   the Write function will take care of CRC, and compression for IDAT.
*/
class Chunk{
    public:
    
    const unsigned int length;
    unsigned char type[4];
    unsigned char *const data;

    Chunk(unsigned int chunkLength, unsigned char chunkType[4] )
    : length(chunkLength), data((new unsigned char[chunkLength+4]())+4)
    {
        for(int i=0;i<4;i++)
        type[i]=chunkType[i];
    }

    ~Chunk(){
        delete[] data;
    }

    //write the chunk into a file using via given file stream
    void Write(ofstream &stream){
    
        //checking for IDAT array         
        if(type[0]=='I' && type[1]=='D' && type[2]=='A' && type[3]=='T')
        {
            ofstream defStream;
            defStream.open("toDeflate",ios::binary);
            defStream.write((char*)data,length);
            defStream.close();

            ShellExecuteA(NULL,"open","deflater","toDeflate",NULL,SW_HIDE);
            
            ifstream in;
            in.open("deflated", ifstream::ate | ifstream::binary);
            while(in.fail()){
              Sleep(1000);
              in.open("deflated", ifstream::ate | ifstream::binary);
            }
            
            unsigned int actualDataLength = in.tellg();
            in.close();
            
            unsigned char actualDataLengthArray[4];
            intToBytes(actualDataLength,actualDataLengthArray);
            stream.write((char*)actualDataLengthArray,4);


            stream.write((char*)type,4);

            ifstream readFile;
            readFile.open("deflated",ifstream::binary);

            char buffer[1024];
            while(!readFile.fail()){
            readFile.read(buffer,1024);
            stream.write(buffer,readFile.gcount());
            }

            readFile.close();

            unsigned char chunkCrc[4];
            intToBytes((unsigned int)(crc((data-4),length+4)),chunkCrc);
            stream.write((char*)chunkCrc,4);

            remove("deflated");
            remove("toDeflate");

        }
        
        else{
            unsigned char arrLength[4];
            intToBytes(length, arrLength);
            stream.write((char*)arrLength,4);
            
            stream.write((char*)type,4);

            stream.write((char*)data,length);

            unsigned char chunkCrc[4];
            intToBytes((unsigned int)(crc((data-4),length+4)),chunkCrc);
            stream.write((char*)chunkCrc,4);
        }
    }
};

//argv[1]: image rgb data (top-left to right-bottom) file's path argv[2]: image width argv[3]: image height argv[4]: output png file's name
int main(int argc, char **argv){

    //set parameters

    char* fileName=argv[1];
    unsigned int imgWidth=strtol(argv[2],nullptr,0), imgHeight=strtol(argv[3],nullptr,0);

    //open stream for output file

    ofstream stream;
    stream.open(argv[4],ios::binary);


    //write png signature

    unsigned char signature[8]={0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A};
    stream.write((char*)signature,8);
    
    //create and write IHDR

    Chunk IHDR(13,(unsigned char*)"IHDR");
    intToBytes(imgWidth,IHDR.data);
    intToBytes(imgHeight,(IHDR.data+4));
    unsigned char IHDRRest[5]={0x08,0x02,0x00,0x00,0x00};
    for(int i=0;i<5;i++)
    (IHDR.data+8)[i]=IHDRRest[i];
    IHDR.Write(stream);
    

    //create and write IDAT

    Chunk IDAT((imgHeight*imgWidth*3)+imgHeight,(unsigned char*)"IDAT");
    
    ifstream readData;
    readData.open(fileName,ifstream::binary);

    for(int i=0;i<imgHeight;i++){
      IDAT.data[(i*imgWidth*3)+i]=0x00;
      unsigned char* j=((IDAT.data)+(i*imgWidth*3)+i+1);
      readData.read((char*)j,imgWidth*3);
    }

    readData.close();

    IDAT.Write(stream);

    //create and write IEND
    Chunk IEND(0,(unsigned char*)"IEND");
    IEND.Write(stream);

    stream.close();

    return 0;
}