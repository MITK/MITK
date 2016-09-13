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
  m_ReturnCode = GOpen("COM7 --baud 115200", &m_GalilSystem);
  return true;
}

bool mitk::GalilMotor::CloseConnection()
{
  if (m_GalilSystem)
  {
    m_ReturnCode = GClose(m_GalilSystem);
    m_GalilSystem = 0;
    return true;
  }
  else
    return false;
}

bool mitk::GalilMotor::TuneToWavelength(double wavelength)
{
  m_ReturnCode = GProgramDownload(m_GalilSystem, "", 0);
  m_ReturnCode = GProgramDownloadFile(m_GalilSystem, "c:/opotek/tuneOPOto700.dmc", 0);
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
  m_ReturnCode = GProgramDownloadFile(m_GalilSystem, "c:/opotek/homeOPO.dmc", 0);
  return true;
}
