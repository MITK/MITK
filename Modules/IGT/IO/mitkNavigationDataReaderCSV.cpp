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

mitk::NavigationDataReaderCSV::NavigationDataReaderCSV()
{

}

mitk::NavigationDataReaderCSV::~NavigationDataReaderCSV()
{

}

mitk::NavigationDataSet::Pointer mitk::NavigationDataReaderCSV::Read(std::string fileName)
{
  mitk::NavigationDataSet::Pointer returnValue = mitk::NavigationDataSet::New(1);
  std::vector<std::string> fileContentLineByLine = GetFileContentLineByLine(fileName);
  for(int i=1; (i<fileContentLineByLine.size()); i++) //skip header so start at 1
  {
    std::vector<mitk::NavigationData::Pointer> currentDatas;
    currentDatas.push_back(GetNavigationDataOutOfOneLine(fileContentLineByLine.at(i)));
    returnValue->AddNavigationDatas(currentDatas);
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

mitk::NavigationData::Pointer mitk::NavigationDataReaderCSV::GetNavigationDataOutOfOneLine(std::string line)
{
  mitk::NavigationData::Pointer returnValue = mitk::NavigationData::New();

  /* NOT FUNCTIONAL BECAUSE IT USES QSTRING WHICH IS NOT AVAILABLE HERE
  QString myLine = QString(line.c_str());

  QStringList myLineList = myLine.split(';');

  mitk::Point3D position;
  mitk::Quaternion orientation;
  bool valid = false;
  double time;

  if (m_Filetype =  mitk::NavigationDataCSVSequentialPlayer::NavigationDataCSV)
    {

    if (myLineList.size() < 10)
      {
        MITK_ERROR << "Error: cannot read line: only found " << myLineList.size() << " fields. Last field: " << myLineList.at(myLineList.size()-1).toStdString()  ;
        returnValue = GetEmptyNavigationData();
        return returnValue;
      }

    time = myLineList.at(1).toDouble();

    if (myLineList.at(2).toStdString() == "1") valid = true;

    position[0] = myLineList.at(3).toDouble();
    position[1] = myLineList.at(4).toDouble();
    position[2] = myLineList.at(5).toDouble();

    orientation[0] = myLineList.at(6).toDouble();
    orientation[1] = myLineList.at(7).toDouble();
    orientation[2] = myLineList.at(8).toDouble();
    orientation[3] = myLineList.at(9).toDouble();
    }
  else
    {
    if (myLineList.size() < 10)
      {
        MITK_ERROR << "Error: cannot read line: only found " << myLineList.size() << " fields. Last field: " << myLineList.at(myLineList.size()-1).toStdString()  ;
        returnValue = GetEmptyNavigationData();
        return returnValue;
      }

    time = myLineList.at(1).toDouble();

    //if (myLineList.at(2).toStdString() == "true")
    //valid-flag wurde hier nicht gespeichert
    valid = true;

    position[0] = myLineList.at(3).toDouble();
    position[1] = myLineList.at(4).toDouble();
    position[2] = myLineList.at(5).toDouble();

    orientation[0] = myLineList.at(6).toDouble();
    orientation[1] = myLineList.at(7).toDouble();
    orientation[2] = myLineList.at(8).toDouble();
    orientation[3] = myLineList.at(9).toDouble();
    }

  returnValue->SetTimeStamp(time);
  returnValue->SetDataValid(valid);
  returnValue->SetPosition(position);
  returnValue->SetOrientation(orientation);
  */

  return returnValue;
}

mitk::NavigationData::Pointer mitk::NavigationDataReaderCSV::GetEmptyNavigationData()
  {
  mitk::NavigationData::Pointer emptyNd = mitk::NavigationData::New();
  mitk::NavigationData::PositionType position;
  mitk::NavigationData::OrientationType orientation(0.0,0.0,0.0,0.0);
  position.Fill(0.0);

  emptyNd->SetPosition(position);
  emptyNd->SetOrientation(orientation);
  emptyNd->SetDataValid(false);
  return emptyNd;
  }
