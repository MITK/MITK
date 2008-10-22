#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char** argv) {
  if ( argc < 4 ) {
    std::cerr << "Usage: " << argv[0] << " infile outfile constname [-b]" << std::endl;
    return EXIT_FAILURE;
  }
  std::ifstream infile(argv[1]);
  std::ofstream outfile(argv[2]);
  std::string buffer;
  if (infile.bad() || infile.eof()) {
    std::cerr << "Can't read " << argv[1] << " for reading!" << std::endl;
    return EXIT_FAILURE;
  }
  infile.seekg (0, std::ios::end);
  int length = infile.tellg();
  infile.seekg (0, std::ios::beg);
  
  

  if (argc == 5 && strcmp(argv[4],"-b") == 0) {
    // convert in binary mode, create stringstream
    outfile << "static std::stringstream " << argv[3] << " ( std::string (\n\"";
    outfile.setf (std::ios::hex,std::ios::basefield);
    outfile.fill('0');
    int nOutputChars = 0;
    while (infile.good()) {
      char c;
      infile.get(c);
      outfile << "\\x"; 
      outfile.width(2);
      unsigned int d = (unsigned char)c; 
      outfile << d;
      if (++nOutputChars > 26) {
        nOutputChars = 0;
        outfile << "\"\n\"";
      }
    }
    outfile.setf (std::ios::dec,std::ios::basefield);
    outfile << "\"," << length  << " ) , std::ios_base::binary|std::ios_base::in); " << std::endl;
  } else {
    outfile << "static const char *" << argv[3] << "[]={\n\"";
    while (infile.good()) {
      std::getline(infile,buffer);
      std::string::iterator it;
      for (it=buffer.begin() ; it < buffer.end() ; it++) {
        char c = *it;    
        if ( c == '"' || c == '\\' ) {
          outfile << '\\';
        }
        outfile << c;
      }
      outfile << '"' << std::endl << '"' ;
    }
    outfile << "\"};" << std::endl;
  }

  if (outfile.good()) {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}

