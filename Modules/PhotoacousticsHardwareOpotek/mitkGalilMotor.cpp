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
m_ComPort(7),
m_ErrorMessage("undefined"),
m_CurrentWavelength(750)
{
  m_GalilSystem = 0;
  m_ReturnCode = G_NO_ERROR;
  LoadResorceFile("opoCalibration.xml", &m_XmlOpoConfiguration);
  TiXmlDocument xmlDoc;

  if (xmlDoc.Parse(m_XmlOpoConfiguration.c_str(), 0, TIXML_ENCODING_UTF8))
  {
    TiXmlElement* root = xmlDoc.FirstChildElement("Motor");
    if (root)
    {
      TiXmlElement* elementCoefficients = root->FirstChildElement("Coefficients");
      TiXmlElement* element = elementCoefficients->FirstChildElement("lambda0");
      m_WavelengthToStepCalibration[0] = std::stod(element->GetText());
      element = elementCoefficients->FirstChildElement("lambda1");
      m_WavelengthToStepCalibration[1] = std::stod(element->GetText());
      element = elementCoefficients->FirstChildElement("lambda2");
      m_WavelengthToStepCalibration[2] = std::stod(element->GetText());
      element = elementCoefficients->FirstChildElement("lambda3");
      m_WavelengthToStepCalibration[3] = std::stod(element->GetText());
      element = elementCoefficients->FirstChildElement("lambda4");
      m_WavelengthToStepCalibration[4] = std::stod(element->GetText());
      element = elementCoefficients->FirstChildElement("lambda5");
      m_WavelengthToStepCalibration[5] = std::stod(element->GetText());
      element = elementCoefficients->FirstChildElement("lambda6");
      m_WavelengthToStepCalibration[6] = std::stod(element->GetText());
      element = elementCoefficients->FirstChildElement("offset");
      m_WavelengthToStepCalibration[7] = std::stod(element->GetText());
    }
  }
  else
  {
    MITK_ERROR << "[GalilMotor Debug] Could not load configuration xml ";
  }

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

bool mitk::GalilMotor::OpenConnection()
{
  MITK_INFO << "[Galil Debug] before GOpen = " << m_ReturnCode << "; m_GalilSystem = " << m_GalilSystem;
  m_ReturnCode = GOpen("COM7 --baud 115200", &m_GalilSystem);
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
  int pos = m_WavelengthToStepCalibration[0]; //+ m_WavelengthToStepCalibration[7];
  pos += m_WavelengthToStepCalibration[1] * wavelength;
  pos += m_WavelengthToStepCalibration[2] * std::pow(wavelength, 2);
  pos += m_WavelengthToStepCalibration[3] * std::pow(wavelength, 3);
  pos += m_WavelengthToStepCalibration[4] * std::pow(wavelength, 4);
  pos += m_WavelengthToStepCalibration[5] * std::pow(wavelength, 5);
  pos += m_WavelengthToStepCalibration[6] * std::pow(wavelength, 6);
  return pos;
}

bool mitk::GalilMotor::TuneToWavelength(double wavelength)
{
  MITK_INFO << "[Galil Debug] after empty GProgramDownload = " << m_ReturnCode << "; m_GalilSystem = " << m_GalilSystem;

  m_ReturnCode = GProgramDownload(m_GalilSystem, "", 0);

  std::string positionCommand;
  positionCommand = "pos = " + std::to_string(this->GetPositionFromWavelength(wavelength)) + "; ";

  std::string galilSlowTuneConf;
  this->LoadResorceFile("configSlowTuneOPO.dmc", &galilSlowTuneConf);
  std::string galilSlowTuneExec;
  this->LoadResorceFile("executeSlowTuneOPO.dmc", &galilSlowTuneExec);

  std::string galilSlowTune = galilSlowTuneConf + positionCommand + galilSlowTuneExec;

  m_ReturnCode = GProgramDownload(m_GalilSystem, galilSlowTune.c_str(), 0);

  //m_ReturnCode = GProgramDownloadFile(m_GalilSystem, "c:/opotek/fastTuneOPO.dmc", 0);/*should be tuneOPOto700*/
  MITK_INFO << "[Galil Debug] after tune GProgramDownloadFile = " << m_ReturnCode << "; m_GalilSystem = " << m_GalilSystem;
  m_ReturnCode = GCmd(m_GalilSystem, "XQ");
  GSleep(3000);
  int val = -1;
  GCmdI(m_GalilSystem, "rt=?", &val);
  MITK_INFO << "[Galil Debug] after tuning execution = " << m_ReturnCode << "; successfulTune = " << val;
  return true;
}

bool mitk::GalilMotor::FastTuneWavelengths(std::vector<double> wavelengthList)
{
  MITK_INFO << "[Galil Debug] after empty GProgramDownload = " << m_ReturnCode << "; m_GalilSystem = " << m_GalilSystem;

  m_ReturnCode = GProgramDownload(m_GalilSystem, "", 0);

  std::string positionCommand;
  positionCommand = "count = 0; idx = 0; DM pos[2]; pos[0] = " + std::to_string(36459/*this->GetPositionFromWavelength(wavelength)*/) + "; "
    + "pos[1] = " + std::to_string(24577/*this->GetPositionFromWavelength(wavelength)*/) + ";";

  std::string galilFastTuneConf;
  this->LoadResorceFile("configSlowTuneOPO.dmc", &galilFastTuneConf);
  std::string galilFastTuneExec;
  this->LoadResorceFile("executeSlowTuneOPO.dmc", &galilFastTuneExec);

  std::string galilFastTune = galilFastTuneConf + positionCommand + galilFastTuneExec;

  m_ReturnCode = GProgramDownload(m_GalilSystem, galilFastTune.c_str(), 0);

  //m_ReturnCode = GProgramDownloadFile(m_GalilSystem, "c:/opotek/fastTuneOPO.dmc", 0);/*should be tuneOPOto700*/
  MITK_INFO << "[Galil Debug] after tune GProgramDownloadFile = " << m_ReturnCode << "; m_GalilSystem = " << m_GalilSystem;
  m_ReturnCode = GCmd(m_GalilSystem, "XQ");
  GSleep(3000);
  int val = -1;
  GCmdI(m_GalilSystem, "rt=?", &val);
  MITK_INFO << "[Galil Debug] after tuning execution = " << m_ReturnCode << "; successfulTune = " << val;
  return true;
}

bool mitk::GalilMotor::Home()
{
  m_ReturnCode = GProgramDownload(m_GalilSystem, "", 0);

  MITK_INFO << "[Galil Debug] after empty GProgramDownload = " << m_ReturnCode << "; m_GalilSystem = " << m_GalilSystem;
  std::string galilProgram;
  this->LoadResorceFile("homeOPO.dmc", &galilProgram);
  //m_ReturnCode = GProgramDownloadFile(m_GalilSystem, "C:/opotek/homeOPO.dmc", 0);
  m_ReturnCode = GProgramDownload(m_GalilSystem, galilProgram.c_str(), 0);

  MITK_INFO << "[Galil Debug] after home GProgramDownloadFile = " << m_ReturnCode << "; m_GalilSystem = " << m_GalilSystem;
  m_ReturnCode = GCmd(m_GalilSystem, "XQ");
  GSleep(10000);
  int val = -1;
  GCmdI(m_GalilSystem, "rh=?", &val);

  MITK_INFO << "[Galil Debug] after home execution = " << m_ReturnCode << "; m_GalilSystem = " << m_GalilSystem << " homed = " << val;

  return true;
}