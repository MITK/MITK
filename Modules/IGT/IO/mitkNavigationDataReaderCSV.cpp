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

#include "mitkNavigationDataReaderCSV.h"
#include <fstream>

mitk::NavigationDataReaderCSV::NavigationDataReaderCSV()
{

}

mitk::NavigationDataReaderCSV::~NavigationDataReaderCSV()
{

}

int mitk::NavigationDataReaderCSV::getNumberOfToolsInLine(std::string line)
{
  std::vector<std::string> tokens=splitLine(line);
 int size = tokens.size();
 int NumOfTools = (size-1)/8;

 if ( (size-1)%8 != 0 )
 {
   MITK_ERROR("mitkNavigationDataReader") << "Illegal csv-file! Unexpected number of columns found! Assuming " << NumOfTools << " tools!";
 }

 return NumOfTools ;
}

std::vector<std::string> mitk::NavigationDataReaderCSV::splitLine(std::string line)
{
   std::vector<std::string> elems;
   std::stringstream ss(line);
   std::string item;
   while (std::getline(ss, item, ';')) {
     elems.push_back(item);
   }
   return elems;
}

mitk::NavigationData::Pointer mitk::NavigationDataReaderCSV::CreateNd(std::string timestamp, std::string valid, std::string X, std::string Y, std::string Z, std::string QX, std::string QY, std::string QZ, std::string QR)
{
  mitk::NavigationData::Pointer result= mitk::NavigationData::New();

  mitk::Point3D position;
  mitk::Quaternion orientation;
  bool isValid = false;
  double time;

  time = StringToDouble(timestamp);

  if (valid == "1") isValid = true;
  else isValid = false;

  position[0] = StringToDouble(X);
  position[1] = StringToDouble(Y);
  position[2] = StringToDouble(Z);

  orientation[0] = StringToDouble(QX);
  orientation[1] = StringToDouble(QY);
  orientation[2] = StringToDouble(QZ);
  orientation[3] = StringToDouble(QR);

  result->SetIGTTimeStamp(time);
  result->SetDataValid(isValid);
  result->SetPosition(position);
  result->SetOrientation(orientation);
  return result;
}

double mitk::NavigationDataReaderCSV::StringToDouble( const std::string& s )
{
  std::istringstream i(s);
  double x;
    if (!(i >> x))
      return 0;
    return x;
}

std::vector<mitk::NavigationData::Pointer> mitk::NavigationDataReaderCSV::parseLine(std::string line, int NumOfTools)
{
  std::vector<std::string> parts= splitLine(line);
  std::vector<mitk::NavigationData::Pointer> result;
  std::string time=  parts[0];



  for (int n= 0; n=NumOfTools; n++)
  {
    mitk::NavigationData::Pointer nd;
    nd = CreateNd(time, parts[n+1],parts[n+2],parts[n+3], parts[n+4], parts[n+5], parts[n+6], parts[n+7], parts[n+8]);
    result.push_back(nd);
  }
  return result;
}


mitk::NavigationDataSet::Pointer mitk::NavigationDataReaderCSV::Read(std::string fileName)
{
  std::vector<std::string> fileContent = GetFileContentLineByLine(fileName);
  int NumOfTools = getNumberOfToolsInLine(fileContent[0]);

  mitk::NavigationDataSet::Pointer returnValue = mitk::NavigationDataSet::New(NumOfTools);

  for (int i = 1; i<fileContent.size(); i++ )
    {
      returnValue->AddNavigationDatas( parseLine( fileContent[i], NumOfTools) );
    }


  return returnValue;
}


std::vector<std::string> mitk::NavigationDataReaderCSV::GetFileContentLineByLine(std::string filename)
{
std::vector<std::string> readData = std::vector<std::string>();

//save old locale
char * oldLocale;
oldLocale = setlocale( LC_ALL, 0 );

//define own locale
std::locale C("C");
setlocale( LC_ALL, "C" );

//read file
std::ifstream file;
file.open(filename.c_str(), std::ios::in);
if (file.good())
    {
    //read out file
    file.seekg(0L, std::ios::beg);  // move to begin of file
    while (! file.eof())
      {
      std::string buffer;
      std::getline(file,buffer);    // read out file line by line
      if (buffer.size() > 0) readData.push_back(buffer);

      }
    }

file.close();

//switch back to old locale
setlocale( LC_ALL, oldLocale );

return readData;
}
