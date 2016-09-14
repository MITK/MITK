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

#include "mitkOpotekLaser.h"
#include "OPOTEK.h"

#include "gclib.h"
#include "gclibo.h"

#include <chrono>
#include <thread>

mitk::OpotekLaser::OpotekLaser() :
m_ConfigurationFilePath(""),
m_MotorComPort(0),
m_PumpLaserComPort(0),
m_ErrorCode(0),
m_WavelengthMin(0),
m_WavelengthMax(0),
m_CurrentWavelengthInNmTenths(7500),
m_IsFlashing(false),
m_IsEmitting(false)
{
  m_ErrorMessages[0] = "No Error";
  m_ErrorMessages[10000] = "No Error";

  m_ErrorMessages[501001] = "OPO Tuning (motors) - No COM ports detected.";
  m_ErrorMessages[501002] = "OPO Tuning (motors) - Cannot initialize controllers. Number of axes is incorrect, power is not on, bad communication.";
  m_ErrorMessages[501003] = "OPO Tuning (motors) - Position out of range.Coerced to limit.";
  m_ErrorMessages[501011] = "OPO Tuning (motors) - The system does not have an automated polarizer.";
  m_ErrorMessages[501012] = "OPO Tuning (motors) - Polarizer not present / detected.";
  m_ErrorMessages[501013] = "OPO Tuning (motors) - The system does not have a motorized attenuator.";
  m_ErrorMessages[501014] = "OPO Tuning (motors) - Attenuator not present / detected.";
  m_ErrorMessages[501015] = "OPO Tuning (motors) - The system does not have motorized harmonic(s).";
  m_ErrorMessages[501016] = "OPO Tuning (motors) - The system does not have a third harmonic generator.";

  m_ErrorMessages[502001] = "Laser - Cannot find pump laser.";
  m_ErrorMessages[502002] = "Laser - Initialize OPO first before initializing pump laser.";
  m_ErrorMessages[502003] = "Laser - Pump laser not initialized.";
  m_ErrorMessages[502004] = "Laser - Q - switch cannot be activated. Check interlocks!";

  m_ErrorMessages[509001] = "General - Motors are not initialized. (Motors needs to be initialized first. Call 'opotek_opo_init' to initialize the motors!).";
  m_ErrorMessages[509002] = "General - Pump laser is not initialized. (The pump laser needs to be initialized in order for this function to work.Call 'opotek_laser_init' to initialize the motors!).";
}

mitk::OpotekLaser::~OpotekLaser()
{
  this->ResetAndRelease();
  MITK_INFO << "[Laser Debug] destroyed that laser";
}

void mitk::OpotekLaser::SetConfigurationPath(std::string configurationPath)
{
  MITK_INFO << configurationPath;
  m_ConfigurationFilePath = new char[configurationPath.length() + 1];
  std::strcpy(m_ConfigurationFilePath, configurationPath.c_str());
  MITK_INFO << m_ConfigurationFilePath;
}

bool mitk::OpotekLaser::Initialize()
{
  OPOTEK_Init(m_ConfigurationFilePath, &m_MotorComPort, &m_PumpLaserComPort, &m_ErrorCode);

  if (m_ErrorCode)
  {
    MITK_ERROR << "[Laser Debug] OPOTEK_Init returned error " << m_ErrorCode << ": \n" << m_ErrorMessages[m_ErrorCode];
    return false;
  }
  if (m_PumpLaserComPort == -1)
  {
    MITK_ERROR << "[Laser Debug] OPOTEK_Init could not find Laser COM Port ";
    return false;
  }

  if (m_MotorComPort == -1)
  {
    MITK_INFO << "[Laser Debug] As expected, OPOTEK_Init could not find Motor COM Port";
  }
  else
    MITK_INFO << "[Laser Debug] OPOTEK_Init motor found here: COM" << m_MotorComPort;

  MITK_INFO << "[Laser Debug] OPOTEK_Init laser found here: COM" << m_PumpLaserComPort;

  OPOTEK_SelectConfiguration(0, &m_WavelengthMin, &m_WavelengthMax, &m_ErrorCode);
  if (m_ErrorCode)
  {
    MITK_ERROR << "[Laser Debug] OPOTEK_SelectConfiguration(0,[...]) Signal configuration returned error " << m_ErrorCode << ": \n" << m_ErrorMessages[m_ErrorCode];
    return false;
  }
  else
    MITK_INFO << "[Laser Debug] Signal wavelength range is " << m_WavelengthMin << "nm to " << m_WavelengthMax << "nm";

  OPOTEK_Laser(0, 0, "", &m_ErrorCode);
  if (m_ErrorCode)
  {
    MITK_ERROR << "[Laser Debug] OPOTEK_Laser(0,[...]) Initialization returned error " << m_ErrorCode << ": \n" << m_ErrorMessages[m_ErrorCode];
    return false;
  }

  OPOTEK_Laser(5, 0, "TRIG EE\n", &m_ErrorCode);
  OPOTEK_Laser(3, 80, "", &m_ErrorCode);

  if (m_ErrorCode)
  {
    MITK_ERROR << "[Laser Debug] OPOTEK_Laser(5,[...]) Trying to set both triggers to external returned error " << m_ErrorCode << ": \n" << m_ErrorMessages[m_ErrorCode];
    return false;
  }

  return true;
}

bool mitk::OpotekLaser::ResetAndRelease()
{
  this->StopQswitching();
  this->StopFlashing();

  OPOTEK_Close(&m_ErrorCode);
  if (m_ErrorCode)
  {
    MITK_ERROR << "[Laser Debug] OPOTEK_Close - Reset and Release returned error " << m_ErrorCode << ": \n" << m_ErrorMessages[m_ErrorCode];
    return false;
  }
  else
    return true;
}

bool mitk::OpotekLaser::TuneToWavelength(int wavelengthInNanoMeterTenths)
{
  MITK_ERROR << "[Laser Debug] OPOTEK_Motor(0,wavelength,[...]) simply does not work #fixme #externaldependency #thirdhashtag";
  return false;
  //MITK_INFO << wavelengthInNanoMeterTenths;
  //OPOTEK_Motor(0, wavelengthInNanoMeterTenths, 0, &m_ErrorCode);
  //if (m_ErrorCode)
  //{
  //  MITK_ERROR << "[Laser Debug] OPOTEK_Motor(0,wavelength,[...]) Tuning returned error " << m_ErrorCode << ": \n" << m_ErrorMessages[m_ErrorCode];
  //  return false;
  //}
  //else
  //{
  //  m_CurrentWavelengthInNmTenths = wavelengthInNanoMeterTenths;
  //  return true;
  //}
}

bool mitk::OpotekLaser::StartFlashing()
{
  OPOTEK_Laser(1, 1, "", &m_ErrorCode);
  if (m_ErrorCode)
  {
    MITK_ERROR << "[Laser Debug] OPOTEK_Laser(1,1,[...]) StartFlashing returned error " << m_ErrorCode << ": \n" << m_ErrorMessages[m_ErrorCode];
    m_IsFlashing = false;
  }
  else
    m_IsFlashing = true;
  return m_IsFlashing;
}

bool mitk::OpotekLaser::StopFlashing()
{
  OPOTEK_Laser(1, 0, "", &m_ErrorCode);
  if (m_ErrorCode)
  {
    MITK_ERROR << "[Laser Debug] OPOTEK_Laser(1,0,[...]) StopFlashing returned error " << m_ErrorCode << ": \n" << m_ErrorMessages[m_ErrorCode];
    m_IsFlashing = true;
  }
  else
    m_IsFlashing = false;
  return !m_IsFlashing;
}

bool mitk::OpotekLaser::StartQswitching()
{
  OPOTEK_Laser(2, 1, "", &m_ErrorCode);
  if (m_ErrorCode)
  {
    MITK_ERROR << "[Laser Debug] OPOTEK_Laser(2,1,[...]) StartQswitching returned error " << m_ErrorCode << ": \n" << m_ErrorMessages[m_ErrorCode];
    m_IsEmitting = false;
  }
  else
    m_IsEmitting = true;
  return m_IsEmitting;
}

bool mitk::OpotekLaser::StopQswitching()
{
  OPOTEK_Laser(2, 0, "", &m_ErrorCode);
  if (m_ErrorCode)
  {
    MITK_ERROR << "[Laser Debug] OPOTEK_Laser(2,0,[...]) StopQswitching returned error " << m_ErrorCode << ": \n" << m_ErrorMessages[m_ErrorCode];
    m_IsEmitting = true;
  }
  else
    m_IsEmitting = false;
  return !m_IsEmitting;
}

int mitk::OpotekLaser::GetMinWavelength()
{
  return (int)(m_WavelengthMin * 10);
}
int mitk::OpotekLaser::GetMaxWavelength()
{
  return (int)(m_WavelengthMax * 10);
}
int mitk::OpotekLaser::GetWavelength()
{
  return m_CurrentWavelengthInNmTenths;
}
bool mitk::OpotekLaser::IsFlashing()
{
  return m_IsFlashing;
}
bool mitk::OpotekLaser::IsEmitting()
{
  return m_IsEmitting;
}