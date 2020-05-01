/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include "mitkNavigationDataReaderCSV.h"
#include <mitkIGTMimeTypes.h>
#include <mitkLocaleSwitch.h>

// STL
#include <fstream>


mitk::NavigationDataReaderCSV::NavigationDataReaderCSV() : AbstractFileReader(
  mitk::IGTMimeTypes::NAVIGATIONDATASETCSV_MIMETYPE(),
  "MITK NavigationData Reader (CSV)")
{
  RegisterService();
}

mitk::NavigationDataReaderCSV::NavigationDataReaderCSV(const mitk::NavigationDataReaderCSV& other) : AbstractFileReader(other)
{
}

mitk::NavigationDataReaderCSV::~NavigationDataReaderCSV()
{
}

mitk::NavigationDataReaderCSV* mitk::NavigationDataReaderCSV::Clone() const
{
  return new NavigationDataReaderCSV(*this);
}

std::vector<itk::SmartPointer<mitk::BaseData>> mitk::NavigationDataReaderCSV::DoRead()
{
  std::vector<std::string> fileContent = GetFileContentLineByLine(GetInputLocation());
  int NumOfTools = getNumberOfToolsInLine(fileContent[0]);

  mitk::NavigationDataSet::Pointer returnValue = mitk::NavigationDataSet::New(NumOfTools);
  std::vector<mitk::BaseData::Pointer> result;
  result.push_back(returnValue.GetPointer());

  // start from line 1 to leave out header
  for (unsigned int i = 1; i<fileContent.size(); i++)
  {
    returnValue->AddNavigationDatas(parseLine(fileContent[i], NumOfTools));
  }

  return result;
}


int mitk::NavigationDataReaderCSV::getNumberOfToolsInLine(std::string line)
{
  std::vector<std::string> tokens=splitLine(line);
 int size = tokens.size();
 int NumOfTools = (size)/9;

 if ( (size)%9 != 0 )
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
  std::vector<std::string> parts = splitLine(line);
  std::vector<mitk::NavigationData::Pointer> result;



  for (int n = 0; n < NumOfTools; n++)
  {
    mitk::NavigationData::Pointer nd;
    int offset = n * 9;
    nd = CreateNd(parts[offset], parts[offset + 1], parts[offset + 2], parts[offset + 3], parts[offset + 4], parts[offset + 5], parts[offset + 6], parts[offset + 7], parts[offset + 8]);
    result.push_back(nd);
  }
  return result;
}


std::vector<std::string> mitk::NavigationDataReaderCSV::GetFileContentLineByLine(std::string filename)
{
std::vector<std::string> readData = std::vector<std::string>();

//define own locale
mitk::LocaleSwitch localeSwitch("C");

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

return readData;
}
