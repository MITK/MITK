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

#include "mitkOphirPyro.h"

#include <chrono>
#include <thread>

mitk::OphirPyro::OphirPyro() :
m_CurrentWavelength(0),
m_DeviceHandle(0),
m_Connected(false),
m_SerialNumber(nullptr)
{
  m_PulseEnergy.clear();
  m_PulseTime.clear();
  m_PulseStatus.clear();
}

mitk::OphirPyro::~OphirPyro()
{
  this->CloseConnection();
  MITK_INFO << "[OphirPyro Debug] destroyed that Pyro";
}

bool mitk::OphirPyro::StartDataAcquisition()
{
  return ophirAPI.StartStream(m_DeviceHandle);
}

bool mitk::OphirPyro::StopDataAcquisition()
{
  return ophirAPI.StopStream(m_DeviceHandle);
}

unsigned int mitk::OphirPyro::GetDataFromSensor()
{
  std::vector<double> newEnergy;
  std::vector<double> newTimestamp;
  std::vector<int> newStatus;
  unsigned int noPackages = ophirAPI.GetData(m_DeviceHandle, &newEnergy, &newTimestamp, &newStatus);
  //newEnergy.resize(noPackages);
  //newTimestamp.resize(noPackages);
  //newStatus.resize(noPackages);

  MITK_INFO << "[Pyro Debug] -- nop:" << noPackages;
  MITK_INFO << "[Pyro Debug] -- newEnergy:" << newEnergy.size();
  if (newEnergy.size() != 0)
    m_PulseEnergy.insert(m_PulseEnergy.end(), newEnergy.begin(), newEnergy.end());
  if (newTimestamp.size() != 0)
    newTimestamp.insert(m_PulseTime.end(), newTimestamp.begin(), newTimestamp.end());
  if (newStatus.size() != 0)
    m_PulseStatus.insert(m_PulseStatus.end(), newStatus.begin(), newStatus.end());
  MITK_INFO << "[Pyro Debug] --";
  return 0;//noPackages;
}

double mitk::OphirPyro::LookupCurrentPulseEnergy()
{
  if (!m_PulseEnergy.empty())
    return m_PulseEnergy.back();
  else
    return 0;
}

double mitk::OphirPyro::GetNextPulseEnergy()
{
  double out = m_PulseEnergy.front();
  m_PulseEnergy.erase(m_PulseEnergy.begin());
  m_PulseTime.erase(m_PulseTime.begin());
  m_PulseStatus.erase(m_PulseStatus.begin());
  return out;
}

double mitk::OphirPyro::LookupCurrentPulseEnergy(double* timestamp, int* status)
{
  *timestamp = m_PulseTime.back();
  *status = m_PulseStatus.back();
  return m_PulseEnergy.back();
}

double mitk::OphirPyro::GetNextPulseEnergy(double* timestamp, int* status)
{
  double out = m_PulseEnergy.front();
  *timestamp = m_PulseTime.front();
  *status = m_PulseStatus.front();
  m_PulseEnergy.erase(m_PulseEnergy.begin());
  m_PulseTime.erase(m_PulseTime.begin());
  m_PulseStatus.erase(m_PulseStatus.begin());
  return out;
}

bool mitk::OphirPyro::OpenConnection()
{
  if (!m_Connected)
  {
    char* m_SerialNumber = ophirAPI.ScanUSB();
    if (m_SerialNumber != 0)
    {
      m_DeviceHandle = ophirAPI.OpenDevice(m_SerialNumber);
      if (m_DeviceHandle != 0)
      {
        m_Connected = true;
        return true;
      }
    }
  }
  return false;
}

bool mitk::OphirPyro::CloseConnection()
{
  if (m_Connected)
  {
    return ophirAPI.CloseDevice(m_DeviceHandle);
  }
  return false;
}