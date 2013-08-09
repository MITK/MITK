/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "mitkDicomRTReader.h"

namespace mitk
{

DicomRTReader::DicomRTReader(){}

DicomRTReader::~DicomRTReader(){}

  int main(int argc, char* argv[])
  {
    std::cout << "main started\n";
    mitk::DicomRTReader::Pointer _DicomRTReader = mitk::DicomRTReader::New();
    char* filename = argv[1];
    int x = _DicomRTReader->ReadContourData(filename);
    std::cout << "main ended\n";
    return 1;
  }

  int DicomRTReader::ReadContourData(char* filename)
  {
    std::cout << "reader started\n";
    const char * x = filename;
    gdcm::Scanner scanner;
    const gdcm::Tag contourData(0x3006,0x0050);
    scanner.AddTag( contourData );

    //gdcm::Scanner::TagToValue const &ttv = scanner.GetMapping(x);
    //std::cout << ttv[contourData];
    std::cout << "reader ended\n";
    return 1;
  }

}