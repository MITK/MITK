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
#include <exception>

mitk::OphirPyro::OphirPyro() :
m_CurrentWavelength(0),
m_DeviceHandle(0),
m_Connected(false),
m_Streaming(false),
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
  if (ophirAPI.StartStream(m_DeviceHandle))
    m_Streaming = true;
  return m_Streaming;
}

bool mitk::OphirPyro::StopDataAcquisition()
{
  if (ophirAPI.StopStream(m_DeviceHandle))
    m_Streaming = false;
  return !m_Streaming;
}

unsigned int mitk::OphirPyro::GetDataFromSensor()
{
  if (m_Streaming)
  {
    std::vector<double> newEnergy;
    std::vector<double> newTimestamp;
    std::vector<int> newStatus;
    MITK_INFO << "DAFUQ";
    unsigned int noPackages = 0;
    try
    {
      noPackages = ophirAPI.GetData(m_DeviceHandle, &newEnergy, &newTimestamp, &newStatus);
      if (noPackages > 0)
      {
        m_PulseEnergy.insert(m_PulseEnergy.end(), newEnergy.begin(), newEnergy.end());
        m_PulseTime.insert(m_PulseTime.end(), newTimestamp.begin(), newTimestamp.end());
        m_PulseStatus.insert(m_PulseStatus.end(), newStatus.begin(), newStatus.end());
      }
    }
    catch (std::exception& ex)
    {
      MITK_INFO << "this is weird: " << ex.what();
    }
    return noPackages;
  }
  return 0;
}

double mitk::OphirPyro::LookupCurrentPulseEnergy()
{
  if (m_Connected)
  {
    if (!m_PulseEnergy.empty())
    {
      MITK_INFO << m_PulseEnergy.size();
      return m_PulseEnergy.back();
    }

    else
      return 0;
  }
}

double mitk::OphirPyro::GetNextPulseEnergy()
{
  if (m_Connected)
  {
    double out = m_PulseEnergy.front();
    m_PulseEnergy.erase(m_PulseEnergy.begin());
    m_PulseTime.erase(m_PulseTime.begin());
    m_PulseStatus.erase(m_PulseStatus.begin());
    return out;
  }
}

double mitk::OphirPyro::LookupCurrentPulseEnergy(double* timestamp, int* status)
{
  if (m_Connected)
  {
    *timestamp = m_PulseTime.back();
    *status = m_PulseStatus.back();
    return m_PulseEnergy.back();
  }
  return 0;
}

double mitk::OphirPyro::GetNextPulseEnergy(double* timestamp, int* status)
{
  if (m_Connected)
  {
    double out = m_PulseEnergy.front();
    *timestamp = m_PulseTime.front();
    *status = m_PulseStatus.front();
    m_PulseEnergy.erase(m_PulseEnergy.begin());
    m_PulseTime.erase(m_PulseTime.begin());
    m_PulseStatus.erase(m_PulseStatus.begin());
    return out;
  }
  return 0;
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
    bool closed = ophirAPI.CloseDevice(m_DeviceHandle);
    if (closed) m_DeviceHandle = 0;
    m_Connected = !closed;
    return closed;
  }
  return false;
}