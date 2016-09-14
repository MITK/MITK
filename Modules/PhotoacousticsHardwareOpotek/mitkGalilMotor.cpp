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

#include "gclib.h"
#include "gclib_errors.h"

#include "gclibo.h"

mitk::GalilMotor::GalilMotor() :
m_SomeFilePath(nullptr),
m_ComPort(7),
m_ErrorMessage("undefined"),
m_CurrentWavelength(750)
{
  m_GalilSystem = 0;
  m_ReturnCode = G_NO_ERROR;
}

mitk::GalilMotor::~GalilMotor()
{
  this->CloseConnection();
  MITK_INFO << "[GalilMotor Debug] destroyed that motor";
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

bool mitk::GalilMotor::TuneToWavelength(double wavelength)
{
  MITK_INFO << "[Galil Debug] after empty GProgramDownload = " << m_ReturnCode << "; m_GalilSystem = " << m_GalilSystem;

  m_ReturnCode = GProgramDownload(m_GalilSystem, "", 0);
  m_ReturnCode = GProgramDownloadFile(m_GalilSystem, "c:/opotek/fastTuneOPO.dmc", 0);/*should be tuneOPOto700*/
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
  m_ReturnCode = GProgramDownload(m_GalilSystem, "", 0);
  m_ReturnCode = GProgramDownloadFile(m_GalilSystem, "c:/opotek/fastTuneOPO.dmc", 0);
  return true;
}

bool mitk::GalilMotor::Home()
{
  m_ReturnCode = GProgramDownload(m_GalilSystem, "", 0);

  MITK_INFO << "[Galil Debug] after empty GProgramDownload = " << m_ReturnCode << "; m_GalilSystem = " << m_GalilSystem;
  m_ReturnCode = GProgramDownloadFile(m_GalilSystem, "C:/opotek/homeOPO.dmc", 0);

  MITK_INFO << "[Galil Debug] after home GProgramDownloadFile = " << m_ReturnCode << "; m_GalilSystem = " << m_GalilSystem;
  m_ReturnCode = GCmd(m_GalilSystem, "XQ");
  GSleep(10000);
  int val = -1;
  GCmdI(m_GalilSystem, "rh=?", &val);

  MITK_INFO << "[Galil Debug] after home execution = " << m_ReturnCode << "; m_GalilSystem = " << m_GalilSystem << " homed = " << val;

  return true;
}