#include <tuple>
#include <fstream>
#include <sys/select.h>
#include <unistd.h>
#include <iostream>
int main(int argc, char** argv) {
  std::fstream file("../resources/room1.txt", std::ios::in|std::ios::out);
  char temp;
  while(true)
  {

      file >> std::noskipws >> temp;
      if(temp == 'G') {
        file.seekp(-1, std::ios::cur);
        file << 'A';
      }
      if( file.eof() ) {
        file.close();
        break;
      }  
  }    
  return 0;
}  