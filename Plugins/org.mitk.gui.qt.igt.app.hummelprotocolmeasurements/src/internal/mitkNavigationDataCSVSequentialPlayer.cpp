/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNavigationDataCSVSequentialPlayer.h"
#include <QString>
#include <QStringList>
#include <iostream>
#include <fstream>

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
  return m_CurrentPos >= static_cast<int>(m_NavigationDatas.size());
}

void mitk::NavigationDataCSVSequentialPlayer::
SetFileName(const std::string& fileName)
{
  this->SetNumberOfIndexedOutputs(1);
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

    if (m_CurrentPos > static_cast<int>(m_NavigationDatas.size()))
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
  std::size_t i = m_HeaderRow
    ? 1  //file has a header row, so it has to be skipped when reading the NavigationDatas
    : 0; //file has no header row, so no need to skip the first row

  for ( ; i < fileContentLineByLine.size(); ++i)
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
  oldLocale = setlocale(LC_ALL, nullptr);

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
    while (!file.eof())
    {
      std::string buffer;
      std::getline(file, buffer);    // read out file line by line

      readData.push_back(buffer);

      ++count; if (count == m_SampleCount) count = 0;
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

  QStringList myLineList = myLine.split(m_SeparatorSign);

  mitk::Point3D position;
  mitk::Quaternion orientation;
  bool valid = false;

  //this is for custom csv files. You have adapt the column numbers to correctly
  //interpret your csv file.
  if (m_Filetype == mitk::NavigationDataCSVSequentialPlayer::ManualLoggingCSV)
  {
    if (myLineList.size() < m_MinNumberOfColumns)
    {
      MITK_ERROR << "Error: cannot read line: only found " << myLineList.size() << " fields. Last field: " << myLineList.at(myLineList.size() - 1).toStdString();
      returnValue = GetEmptyNavigationData();
      return returnValue;
    }

    valid = true; //if no valid flag is given: simply set to true


    //############# Variant for the Aurora measurements ###############
    //#############   (CUSTOM .csv files from MITK)     ###############

    position[0] = myLineList.at(3).toDouble();
    position[1] = myLineList.at(4).toDouble();
    position[2] = myLineList.at(5).toDouble();

    orientation[0] = myLineList.at(6).toDouble(); //qx
    orientation[1] = myLineList.at(7).toDouble(); //qy
    orientation[2] = myLineList.at(8).toDouble(); //qz
    orientation[3] = myLineList.at(9).toDouble(); //qr

    if(!m_RightHanded) //MITK uses a right handed coordinate system, so the position needs to be converted
    {
        position[0] = position[0]*(-1);
    }

    if (m_UseQuats) //Use Quaternions to construct the orientation of the NavigationData
    {
        orientation[0] = myLineList.at(m_Qx).toDouble(); //qx
        orientation[1] = myLineList.at(m_Qy).toDouble(); //qy
        orientation[2] = myLineList.at(m_Qz).toDouble(); //qz
        orientation[3] = myLineList.at(m_Qr).toDouble(); //qr
    }
    else //Use the Euler Angles to construct the orientation of the NavigationData
    {
        double azimuthAngle;
        double elevationAngle;
        double rollAngle;
        if(m_Azimuth < 0) //azimuth is not defined so set him to zero
        {
            azimuthAngle = 0;
        }
        else
        {
            azimuthAngle = myLineList.at(m_Azimuth).toDouble();
        }
        if(m_Elevation < 0)// elevation is not defined so set him to zero
        {
            elevationAngle = 0;
        }
        else
        {
            elevationAngle = myLineList.at(m_Elevation).toDouble();
        }
        if(m_Roll < 0) //roll is not defined so set him to zero
        {
            rollAngle = 0;
        }
        else
        {
            rollAngle = myLineList.at(m_Roll).toDouble();
        }


        if (!m_EulersInRadiants) //the Euler Angles are in Degrees but MITK uses radiants so they need to be converted
        {
            azimuthAngle = azimuthAngle / 180 * itk::Math::pi;
            elevationAngle = elevationAngle / 180 * itk::Math::pi;
            rollAngle = rollAngle / 180 * itk::Math::pi;
        }
        vnl_quaternion<double> eulerQuat(rollAngle, elevationAngle, azimuthAngle);
        orientation = eulerQuat;
    }

    if(!m_RightHanded) //MITK uses a right handed coordinate system, so the orientation needs to be converted
    {
      //code block for conversion from left-handed to right-handed
      mitk::Quaternion linksZuRechtsdrehend;
      double rotationAngle = -itk::Math::pi;
      double rotationAxis[3];
      rotationAxis[0] = 0;
      rotationAxis[1] = 0;
      rotationAxis[2] = 1;

      linksZuRechtsdrehend[3] = cos(rotationAngle / 2);
      linksZuRechtsdrehend[0] = rotationAxis[0] * sin(rotationAngle / 2);
      linksZuRechtsdrehend[1] = rotationAxis[1] * sin(rotationAngle / 2);
      linksZuRechtsdrehend[2] = rotationAxis[2] * sin(rotationAngle / 2);

      orientation = orientation * linksZuRechtsdrehend;
    }

  }
  //this is for MITK csv files that have been recorded with the MITK
  //navigation data recorder. You can also use the navigation data player
  //class from the MITK-IGT module instead.
  else if (m_Filetype == mitk::NavigationDataCSVSequentialPlayer::NavigationDataCSV)
  {
    if (myLineList.size() < 8)
    {
      MITK_ERROR << "Error: cannot read line: only found " << myLineList.size() << " fields. Last field: " << myLineList.at(myLineList.size() - 1).toStdString();
      returnValue = GetEmptyNavigationData();
      return returnValue;
    }

    if (myLineList.at(3).toStdString() == "1") valid = true;

    position[0] = myLineList.at(2).toDouble();
    position[1] = myLineList.at(3).toDouble();
    position[2] = myLineList.at(4).toDouble();

    orientation[0] = myLineList.at(5).toDouble(); //qx
    orientation[1] = myLineList.at(6).toDouble(); //qy
    orientation[2] = myLineList.at(7).toDouble(); //qz
    orientation[3] = myLineList.at(8).toDouble(); //qr
  }

  returnValue->SetDataValid(valid);
  returnValue->SetPosition(position);
  returnValue->SetOrientation(orientation);

  return returnValue;
}
void mitk::NavigationDataCSVSequentialPlayer::SetOptions(bool rightHanded, char separatorSign, int sampleCount, bool headerRow, int xPos, int yPos,
                                                         int zPos, bool useQuats, int qx, int qy, int qz, int qr, int azimuth, int elevation, int roll,
                                                         bool eulerInRadiants, int minNumberOfColumns)
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
    m_MinNumberOfColumns = minNumberOfColumns;
}
