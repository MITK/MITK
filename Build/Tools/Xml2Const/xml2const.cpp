#include <iostream>
#include <fstream>

int main(int argc, char** argv) {
  if ( argc != 4 ) {
    std::cerr << "Usage: " << argv[0] << " infile outfile constname" << std::endl;
    return EXIT_FAILURE;
  }
  std::ifstream infile(argv[1]);
  std::ofstream outfile(argv[2]);
  std::string buffer;
  if (infile.bad() || infile.eof()) {
    std::cerr << "Can't read " << argv[1] << " for reading!" << std::endl;
    return EXIT_FAILURE;
  }
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
  if (outfile.good()) {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}
