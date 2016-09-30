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
#define _USE_MATH_DEFINES
#include "mitkNavigationDataCSVSequentialPlayer.h"
#include <QString>
#include <QStringList>
#include <iostream>
#include <fstream>
#include <math.h>

mitk::NavigationDataCSVSequentialPlayer::NavigationDataCSVSequentialPlayer()
  : mitk::NavigationDataPlayerBase()
{
  m_NavigationDatas = std::vector<mitk::NavigationData::Pointer>();
  m_CurrentPos = 0;
  m_Filetype = mitk::NavigationDataCSVSequentialPlayer::ManualLoggingCSV;

}

mitk::NavigationDataCSVSequentialPlayer::~NavigationDataCSVSequentialPlayer()
{
}

bool mitk::NavigationDataCSVSequentialPlayer::IsAtEnd()
{
  if (m_CurrentPos >= m_NavigationDatas.size()) return true;
  else return false;
}

void mitk::NavigationDataCSVSequentialPlayer::
SetFileName(const std::string& fileName)
{
  this->SetNumberOfOutputs(1);
  FillOutputEmpty(0);

  MITK_INFO << "Reading file: " << fileName;
  m_NavigationDatas = GetNavigationDatasFromFile(fileName);

  this->Modified();
}

void mitk::NavigationDataCSVSequentialPlayer::FillOutputEmpty(int number)
{
  this->SetNthOutput(number, GetEmptyNavigationData());
}

mitk::NavigationData::Pointer mitk::NavigationDataCSVSequentialPlayer::GetEmptyNavigationData()
{
  mitk::NavigationData::Pointer emptyNd = mitk::NavigationData::New();
  mitk::NavigationData::PositionType position;
  mitk::NavigationData::OrientationType orientation(0.0, 0.0, 0.0, 0.0);
  position.Fill(0.0);

  emptyNd->SetPosition(position);
  emptyNd->SetOrientation(orientation);
  emptyNd->SetDataValid(false);
  return emptyNd;
}
int mitk::NavigationDataCSVSequentialPlayer::GetNumberOfSnapshots()
{
  return m_NavigationDatas.size();
}
void mitk::NavigationDataCSVSequentialPlayer::GenerateData()
{
  for (unsigned int index = 0; index < this->GetNumberOfOutputs(); index++)
  {
    mitk::NavigationData* output = this->GetOutput(index);

    if (m_CurrentPos > m_NavigationDatas.size())
    {
      FillOutputEmpty(index);
      return;
    }

    output->Graft(this->m_NavigationDatas.at(m_CurrentPos));
    m_CurrentPos++;
  }
}

void mitk::NavigationDataCSVSequentialPlayer::UpdateOutputInformation()
{
  this->Modified();  // make sure that we need to be updated
  Superclass::UpdateOutputInformation();
}

std::vector<mitk::NavigationData::Pointer> mitk::NavigationDataCSVSequentialPlayer::GetNavigationDatasFromFile(std::string filename)
{
  std::vector<mitk::NavigationData::Pointer> returnValue = std::vector<mitk::NavigationData::Pointer>();
  std::vector<std::string> fileContentLineByLine = GetFileContentLineByLine(filename);
  for (int i = 1; (i < fileContentLineByLine.size()); i++) //skip header so start at 1
  {
    returnValue.push_back(GetNavigationDataOutOfOneLine(fileContentLineByLine.at(i)));
  }

  return returnValue;
}

std::vector<std::string> mitk::NavigationDataCSVSequentialPlayer::GetFileContentLineByLine(std::string filename)
{
  std::vector<std::string> readData = std::vector<std::string>();

  //save old locale
  char * oldLocale;
  oldLocale = setlocale(LC_ALL, 0);

  //define own locale
  std::locale C("C");
  setlocale(LC_ALL, "C");

  //read file
  std::ifstream file;
  file.open(filename.c_str(), std::ios::in);
  if (file.good())
  {
    //read out file
    file.seekg(0L, std::ios::beg);  // move to begin of file

    int count = 0;
    //int count2 = 0;
    while (!file.eof())
    {
      std::string buffer;
      std::getline(file, buffer);    // read out file line by line

      //for Polhemus tracker: just take every 24th sample
      if (count == 0) if (buffer.size() > 0)
      {
        //MITK_INFO << "read(" << count2 << "): " << buffer.substr(0,30);
        //count2++;
        readData.push_back(buffer);
      }

      count++; if (count == 24) count = 0;
    }
  }

  file.close();

  //switch back to old locale
  setlocale(LC_ALL, oldLocale);

  return readData;
}

mitk::NavigationData::Pointer mitk::NavigationDataCSVSequentialPlayer::GetNavigationDataOutOfOneLine(std::string line)
{
  mitk::NavigationData::Pointer returnValue = mitk::NavigationData::New();

  QString myLine = QString(line.c_str());

  QStringList myLineList = myLine.split(',');

  mitk::Point3D position;
  mitk::Quaternion orientation;
  bool valid = false;
  double time;

  //this is for custom csv files. You have adapt the column numbers to correctly
  //interpret your csv file.
  if (m_Filetype = mitk::NavigationDataCSVSequentialPlayer::ManualLoggingCSV)
  {
    if (myLineList.size() < 10)
    {
      MITK_ERROR << "Error: cannot read line: only found " << myLineList.size() << " fields. Last field: " << myLineList.at(myLineList.size() - 1).toStdString();
      returnValue = GetEmptyNavigationData();
      return returnValue;
    }

    valid = true; //if no valid flag is given: simply set to true


    //############# Variant for the Aurora measurements ###############
    //#############   (CUSTOM .csv files from MITK)     ###############
    /*
    position[0] = myLineList.at(3).toDouble();
    position[1] = myLineList.at(4).toDouble();
    position[2] = myLineList.at(5).toDouble();

    orientation[0] = myLineList.at(6).toDouble(); //qx
    orientation[1] = myLineList.at(7).toDouble(); //qy
    orientation[2] = myLineList.at(8).toDouble(); //qz
    orientation[3] = myLineList.at(9).toDouble(); //qr
    */

    //Variant for the polhemus measurements in August 2016
    //(.csv files from the polhemus software)

    //Important: due to the documentation, Polhemus uses
    //a left handed coordinate system while MITK uses a
    //right handed. A conversion is not included in this
    //read in method yet, because this is not required
    //for this special rotation evaliation (no matter
    //if it turns 11.25 degree to left or right). For
    //other usage this might be important to adapt!


    position[0] = myLineList.at(4).toDouble();
    position[1] = myLineList.at(5).toDouble();
    position[2] = myLineList.at(6).toDouble();


    //Doesn't work... don't know how to interpret the
    //Polhemus quaternions. They are seem to different
    //different to other quaternions (NDI, Claron, etc.)
    //http://www.mathepedia.de/Quaternionen.aspx

    /*
    double qr = myLineList.at(7).toDouble();
    double qx = myLineList.at(8).toDouble();
    double qy = myLineList.at(9).toDouble();
    double qz = myLineList.at(10).toDouble();

    vnl_quaternion<double> newQuat(qx, qy, qz, qr);

    orientation = newQuat;
    orientation.normalize();*/

    /*
    orientation[3] = qr;  //qr
    orientation[0] = qx;  //qx
    orientation[1] = qy;  //qy
    orientation[2] = qz;  //qz

    orientation.normalize();
    */



    //Using Euler angles instead does work
    //azimuth: rotation about Z axis of reference frame
    double azimuthAngle = (myLineList.at(11).toDouble() / 180 * M_PI);
    //elevation: rotation about Y' axis (transformed Y axis of sonsor frame)
    double elevationAngle = (myLineList.at(12).toDouble() / 180 * M_PI);
    //roll: rotation about X axis of sensor frame
    double rollAngle = (myLineList.at(13).toDouble() / 180 * M_PI);
    vnl_quaternion<double> eulerQuat(rollAngle, elevationAngle, azimuthAngle);
    orientation = eulerQuat;


    /*
    //code block for conversion from axis-angular representation
    double rotationAngle =  myLineList.at(7).toDouble();
    double rotationAxis[3];
    rotationAxis[0] = myLineList.at(8).toDouble();
    rotationAxis[1] = myLineList.at(9).toDouble();
    rotationAxis[2] = myLineList.at(10).toDouble();

    double betragRotationAxis = sqrt(pow(rotationAxis[0], 2) + pow(rotationAxis[1], 2) + pow(rotationAxis[2], 2));
    rotationAngle /= betragRotationAxis;
    rotationAxis[0] /= betragRotationAxis;
    rotationAxis[1] /= betragRotationAxis;
    rotationAxis[2] /= betragRotationAxis;


    double qr = cos(rotationAngle/2);
    double qx = rotationAxis[0] * sin(rotationAngle/2);
    double qy = rotationAxis[1] * sin(rotationAngle/2);
    double qz = rotationAxis[1] * sin(rotationAngle/2);
    */



    /*
    //code block for conversion from left-handed to right-handed
    mitk::Quaternion linksZuRechtsdrehend;
    double rotationAngle = -M_PI;
    double rotationAxis[3];
    rotationAxis[0] = 0;
    rotationAxis[1] = 0;
    rotationAxis[2] = 1;

    linksZuRechtsdrehend[3] = cos(rotationAngle / 2);
    linksZuRechtsdrehend[0] = rotationAxis[0] * sin(rotationAngle / 2);
    linksZuRechtsdrehend[1] = rotationAxis[1] * sin(rotationAngle / 2);
    linksZuRechtsdrehend[2] = rotationAxis[2] * sin(rotationAngle / 2);

    orientation = orientation * linksZuRechtsdrehend;
    */

  }
  //this is for MITK csv files that have been recorded with the MITK
  //navigation data recorder. You can also use the navigation data player
  //class from the MITK-IGT module instead.
  else if (m_Filetype = mitk::NavigationDataCSVSequentialPlayer::NavigationDataCSV)
  {
    if (myLineList.size() < 8)
    {
      MITK_ERROR << "Error: cannot read line: only found " << myLineList.size() << " fields. Last field: " << myLineList.at(myLineList.size() - 1).toStdString();
      returnValue = GetEmptyNavigationData();
      return returnValue;
    }

    time = myLineList.at(2).toDouble();

    if (myLineList.at(3).toStdString() == "1") valid = true;

    position[0] = myLineList.at(2).toDouble();
    position[1] = myLineList.at(3).toDouble();
    position[2] = myLineList.at(4).toDouble();

    orientation[0] = myLineList.at(5).toDouble(); //qx
    orientation[1] = myLineList.at(6).toDouble(); //qy
    orientation[2] = myLineList.at(7).toDouble(); //qz
    orientation[3] = myLineList.at(8).toDouble(); //qr
  }

  //returnValue->SetTimeStamp(time); //DOES NOT WORK ANY MORE... CANNOT SET TIME TO itk::timestamp CLASS
  returnValue->SetDataValid(valid);
  returnValue->SetPosition(position);
  returnValue->SetOrientation(orientation);

  return returnValue;
}
void mitk::NavigationDataCSVSequentialPlayer::SetOptions(bool rightHanded, std::string separatorSign, int sampleCount, bool headerRow, int xPos, int yPos,
                                                         int zPos, bool useQuats, int qx, int qy, int qz, int qr, int azimuth, int elevation, int roll, bool eulerInRadiants)
{
    m_RightHanded = rightHanded;
    m_SeparatorSign = separatorSign;
    m_SampleCount = sampleCount;
    m_HeaderRow = headerRow;
    m_XPos = xPos;
    m_YPos = yPos;
    m_ZPos = zPos;
    m_UseQuats = useQuats;
    m_Qx = qx;
    m_Qy = qy;
    m_Qz = qz;
    m_Qr = qr;
    m_Azimuth = azimuth;
    m_Elevation = elevation;
    m_Roll = roll;
    m_EulersInRadiants = eulerInRadiants;
}
