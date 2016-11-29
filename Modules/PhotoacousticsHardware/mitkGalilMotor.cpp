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

#include "mitkGalilMotor.h"

#include <chrono>
#include <thread>

#include <math.h>

#include "gclib.h"
#include "gclib_errors.h"

#include "gclibo.h"

mitk::GalilMotor::GalilMotor() :
m_ComPort(0),
m_BaudRate(0),
m_ErrorMessage("undefined"),
m_CurrentWavelength(0),
m_HomePosition(32561)
{
}

mitk::GalilMotor::~GalilMotor()
{
  this->CloseConnection();
  MITK_INFO << "[GalilMotor Debug] destroyed that motor";
}

void mitk::GalilMotor::LoadResorceFile(std::string filename, std::string* lines)
{
  us::ModuleResource resorceFile = us::GetModuleContext()->GetModule()->GetResource(filename);
  std::string line;
  if (resorceFile.IsValid() && resorceFile.IsFile())
  {
    us::ModuleResourceStream stream(resorceFile);
    while (std::getline(stream, line))
    {
      *lines = *lines + line + "\n";
    }
  }
  else
  {
    MITK_ERROR << "Resource file was not valid";
  }
}

bool mitk::GalilMotor::OpenConnection(std::string configuration)
{
  m_GalilSystem = 0;
  m_ReturnCode = G_NO_ERROR;
  LoadResorceFile(configuration + ".xml", &m_XmlOpoConfiguration);
  TiXmlDocument xmlDoc;

  if (xmlDoc.Parse(m_XmlOpoConfiguration.c_str(), 0, TIXML_ENCODING_UTF8))
  {
    TiXmlElement* root = xmlDoc.FirstChildElement("Motor");
    if (root)
    {
      TiXmlElement* elementNode = root->FirstChildElement("Coefficients");
      TiXmlElement* element = elementNode->FirstChildElement("lambda0");
      m_WavelengthToStepCalibration[0] = std::stod(element->GetText());
      element = elementNode->FirstChildElement("lambda1");
      m_WavelengthToStepCalibration[1] = std::stod(element->GetText());
      element = elementNode->FirstChildElement("lambda2");
      m_WavelengthToStepCalibration[2] = std::stod(element->GetText());
      element = elementNode->FirstChildElement("lambda3");
      m_WavelengthToStepCalibration[3] = std::stod(element->GetText());
      element = elementNode->FirstChildElement("lambda4");
      m_WavelengthToStepCalibration[4] = std::stod(element->GetText());
      element = elementNode->FirstChildElement("lambda5");
      m_WavelengthToStepCalibration[5] = std::stod(element->GetText());
      element = elementNode->FirstChildElement("lambda6");
      m_WavelengthToStepCalibration[6] = std::stod(element->GetText());
      element = elementNode->FirstChildElement("home");
      m_HomePosition = std::stod(element->GetText());
      elementNode = root->FirstChildElement("Signal");
      element = elementNode->FirstChildElement("Signal-low");
      m_MinWavelength = std::stod(element->GetText());
      element = elementNode->FirstChildElement("Signal-default");
      m_CurrentWavelength = std::stod(element->GetText());
      element = elementNode->FirstChildElement("Signal-high");
      m_MaxWavelength = std::stod(element->GetText());
      elementNode = root->FirstChildElement("Seriell");
      element = elementNode->FirstChildElement("PortNumber");
      m_ComPort = std::stoi(element->GetText());
      element = elementNode->FirstChildElement("Baud");
      m_BaudRate = std::stoi(element->GetText());
    }
  }
  else
  {
    MITK_ERROR << "[GalilMotor Debug] Could not load configuration xml ";
  }
  std::string openCommand("COM" + std::to_string(m_ComPort) + " --baud " + std::to_string(m_BaudRate) + " --subscribe ALL --direct");
  MITK_INFO << "[Galil Debug] before GOpen(" << openCommand << ") = " << m_ReturnCode << "; m_GalilSystem = " << m_GalilSystem;
  m_ReturnCode = GOpen(openCommand.c_str(), &m_GalilSystem);
  MITK_INFO << "[Galil Debug] after GOpen = " << m_ReturnCode << "; m_GalilSystem = " << m_GalilSystem;
  this->Home();
  return true;
}

bool mitk::GalilMotor::CloseConnection()
{
  if (m_GalilSystem)
  {
    this->Home();
    m_ReturnCode = GClose(m_GalilSystem);
    m_GalilSystem = 0;
    return true;
  }
  else
    return false;
}

int mitk::GalilMotor::GetPositionFromWavelength(double wavelength)
{
  double posDouble = 0;
  posDouble = m_WavelengthToStepCalibration[0] - m_HomePosition;
  posDouble += (m_WavelengthToStepCalibration[1] * wavelength);
  posDouble += (m_WavelengthToStepCalibration[2] * std::pow(wavelength, 2));
  posDouble += (m_WavelengthToStepCalibration[3] * std::pow(wavelength, 3));
  posDouble += (m_WavelengthToStepCalibration[4] * std::pow(wavelength, 4));
  posDouble += (m_WavelengthToStepCalibration[5] * std::pow(wavelength, 5));
  posDouble += (m_WavelengthToStepCalibration[6] * std::pow(wavelength, 6));
  int pos = posDouble;
  return pos;
}

bool mitk::GalilMotor::TuneToWavelength(double wavelength, bool isRecalibrating)
{
  std::string positionCommand;
  if (!isRecalibrating)
  {
    positionCommand = "pos=" + std::to_string(this->GetPositionFromWavelength(wavelength));
  }
  else
  {
    int posAbs = wavelength - m_HomePosition;
    positionCommand = "pos=" + std::to_string(posAbs);
  }

  m_ReturnCode = GCmd(m_GalilSystem, "AB"); // Abort any running programs
  MITK_INFO << "[Galil Debug] after AB: " << m_ReturnCode << "";
  m_ReturnCode = GCmd(m_GalilSystem, "BV"); // Burn all variables
  MITK_INFO << "[Galil Debug] after BV: " << m_ReturnCode << "";
  m_ReturnCode = GProgramDownload(m_GalilSystem, "", 0); // Erase the program on the system
  MITK_INFO << "[Galil Debug] after empty GProgramDownload: " << m_ReturnCode << "";

  m_ReturnCode = GCmd(m_GalilSystem, positionCommand.c_str());
  MITK_INFO << "[Galil Debug] after sending tune position(" << positionCommand << "): " << m_ReturnCode << "";

  std::string galilProgramSTUNE;
  this->LoadResorceFile("STUNE.dmc", &galilProgramSTUNE);
  m_ReturnCode = GProgramDownload(m_GalilSystem, galilProgramSTUNE.c_str(), 0);
  MITK_INFO << "[Galil Debug] after STUNE progam: " << m_ReturnCode << galilProgramSTUNE;

  GSleep(10);
  m_ReturnCode = GCmd(m_GalilSystem, "XQ#STUNE");
  MITK_INFO << "[Galil Debug] after sending XQ#STUNE = " << m_ReturnCode;
  GSleep(10000);
  int success, pos = -1;
  m_ReturnCode = GCmdI(m_GalilSystem, "suc=?", &success);
  MITK_INFO << "[Galil Debug] after asking suc=? = " << m_ReturnCode << "; successfulTune = " << success;
  m_ReturnCode = GCmdI(m_GalilSystem, "pos=?", &pos);
  MITK_INFO << "[Galil Debug] after asking pos=? = " << m_ReturnCode << "; pos = " << pos;
  if (success == 1)
  {
    m_CurrentWavelength = wavelength;
    return true;
  }
  else
    return false;
}

bool mitk::GalilMotor::FastTuneWavelengths(std::vector<double> wavelengthList)
{
  m_ReturnCode = GCmd(m_GalilSystem, "AB"); // Abort any running programs
  GSleep(1000);
  MITK_INFO << "[Galil Debug] after AB: " << m_ReturnCode << "";
  m_ReturnCode = GCmd(m_GalilSystem, "BV"); // Burn all variables
  GSleep(2000);
  MITK_INFO << "[Galil Debug] after BV: " << m_ReturnCode << "";

  m_ReturnCode = GProgramDownload(m_GalilSystem, "", 0); // Erase the program on the system
  MITK_INFO << "[Galil Debug] after empty GProgramDownload: " << m_ReturnCode << "";
  GSleep(1000);
  std::string positionsCommand("#AUTO\n#PARA\nDA pos[0]\npoints = "+std::to_string(wavelengthList.size())+"\nDM pos[" + std::to_string(wavelengthList.size()) + "];\n");

  for (int wavelengthIterator = 0; wavelengthIterator < wavelengthList.size(); wavelengthIterator++)
  {
    positionsCommand += "pos[" + std::to_string(wavelengthIterator) + "]=" + std::to_string(this->GetPositionFromWavelength(wavelengthList[wavelengthIterator])) + "\n";
  }
  positionsCommand += "EN\n";

  m_ReturnCode = GProgramDownload(m_GalilSystem, positionsCommand.c_str(), 0);
  MITK_INFO << "[Galil Debug] after sending positions command(" << positionsCommand << "): " << m_ReturnCode << "";
  GSleep(1000);
  m_ReturnCode = GCmd(m_GalilSystem, "XQ"); // FTUNE
  MITK_INFO << "[Galil Debug] after asking XQ#PARA = " << m_ReturnCode;
  m_ReturnCode = GProgramDownload(m_GalilSystem, "", 0); // Erase the program on the system
  MITK_INFO << "[Galil Debug] after empty GProgramDownload: " << m_ReturnCode << "";
  GSleep(1000);
  std::string galilProgramFTUNE;
  this->LoadResorceFile("FTUNE.dmc", &galilProgramFTUNE);
  m_ReturnCode = GProgramDownload(m_GalilSystem, galilProgramFTUNE.c_str(), 0);
  MITK_INFO << "[Galil Debug] after FTUNE progam upload: " << m_ReturnCode << "";

  GSleep(1000);
  m_ReturnCode = GCmd(m_GalilSystem, "XQ"); // FTUNE
  MITK_INFO << "[Galil Debug] after asking XQ#FTUNE = " << m_ReturnCode;
  GSleep(10000);
  int count = -1;
  m_ReturnCode = GCmdI(m_GalilSystem, "count=?", &count);
  MITK_INFO << "[Galil Debug] cycles = " << count;
  m_ReturnCode = GCmdI(m_GalilSystem, "pos=?", &count);
  MITK_INFO << "[Galil Debug] pos = " << count;
  m_ReturnCode = GCmdI(m_GalilSystem, "points=?", &count);
  MITK_INFO << "[Galil Debug] points = " << count;

  if (count > 0)
    return true;
  else
    return false;
}

bool mitk::GalilMotor::Home()
{
  m_ReturnCode = GCmd(m_GalilSystem, "AB"); // Abort any running programs
  MITK_INFO << "[Galil Debug] after AB: " << m_ReturnCode << "";
  GSleep(1000);
  m_ReturnCode = GCmd(m_GalilSystem, "BV"); // Burn all variables
  MITK_INFO << "[Galil Debug] after BV: " << m_ReturnCode << "";
  GSleep(1000);
  m_ReturnCode = GProgramDownload(m_GalilSystem, "", 0); // Erase the program on the system
  MITK_INFO << "[Galil Debug] after empty GProgramDownload: " << m_ReturnCode << "";

  std::string galilProgram;
  this->LoadResorceFile("GHOME.dmc", &galilProgram);

  m_ReturnCode = GProgramDownload(m_GalilSystem, galilProgram.c_str(), 0);
  MITK_INFO << "[Galil Debug] after home GProgramDownloadFile = " << m_ReturnCode << "; m_GalilSystem = " << m_GalilSystem;

  m_ReturnCode = GCmd(m_GalilSystem, "XQ"); // HOME
  GSleep(15000);
  int val = -2;
  m_ReturnCode = GCmdI(m_GalilSystem, "suc=?", &val);

  MITK_INFO << "[Galil Debug] after home execution = " << m_ReturnCode << "; m_GalilSystem = " << m_GalilSystem << " homed = " << val;

  return true;
}

double mitk::GalilMotor::GetMinWavelength()
{
  return m_MinWavelength;
}

double mitk::GalilMotor::GetMaxWavelength()
{
  return m_MaxWavelength;
}

double mitk::GalilMotor::GetCurrentWavelength()
{
  return m_CurrentWavelength;
}