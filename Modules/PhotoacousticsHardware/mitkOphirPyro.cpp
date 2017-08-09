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
#include <ctime>
#include <thread>
#include <exception>

mitk::OphirPyro::OphirPyro() :
m_CurrentWavelength(0),
m_DeviceHandle(0),
m_Connected(false),
m_Streaming(false),
m_SerialNumber(nullptr),
m_GetDataThread(),
m_ImagePyroDelay(0),
m_EnergyMultiplicator(60000)
{
  m_PulseEnergy.clear();
  m_PulseTime.clear();
  m_PulseStatus.clear();
  m_TimeStamps.clear();
}

mitk::OphirPyro::~OphirPyro()
{
  if (m_Connected)
  {
    this->CloseConnection();
    if (m_GetDataThread.joinable())
    {
      m_GetDataThread.join();
      MITK_INFO << "[OphirPyro Debug] joined data thread";
    }
  }
  MITK_INFO << "[OphirPyro Debug] destroying that Pyro";
  /* cleanup thread */
}

bool mitk::OphirPyro::StartDataAcquisition()
{
  if (ophirAPI.StartStream(m_DeviceHandle))
  {
    m_Streaming = true;
    m_GetDataThread = std::thread(&mitk::OphirPyro::GetDataFromSensorThread, this);
  }
  return m_Streaming;
}

// this is just a little function to set the filenames below right
inline void replaceAll(std::string& str, const std::string& from, const std::string& to) {
  if (from.empty())
    return;
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
  }
}

void mitk::OphirPyro::SaveCsvData()
{
  // get the time and date, put them into a nice string and create a folder for the images
  time_t time = std::time(nullptr);
  time_t* timeptr = &time;
  std::string currentDate = std::ctime(timeptr);
  replaceAll(currentDate, ":", "-");
  currentDate.pop_back();

  std::string pathTS = "c:\\ImageData\\" + currentDate + " pyro-ts" + ".csv";

  std::ofstream timestampFile;
  timestampFile.open(pathTS);

  timestampFile << ",timestamp,PulseEnergy,PulseTime";

  int currentSize = m_TimeStamps.size();

  for (int index = 0; index < currentSize; ++index)
  {
    timestampFile << "\n" << index << "," << m_TimeStamps.at(index) << ","<< m_PulseEnergySaved.at(index) << "," << (long)m_PulseTimeSaved.at(index);
  }
  timestampFile.close();
}

void mitk::OphirPyro::SaveData()
{
  SaveCsvData();
}

bool mitk::OphirPyro::StopDataAcquisition()
{
  if (ophirAPI.StopStream(m_DeviceHandle))
    m_Streaming = false;

  SaveCsvData();
  MITK_INFO << "[OphirPyro Debug] m_Streaming = "<< m_Streaming;
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  if (m_GetDataThread.joinable())
  {
    m_GetDataThread.join();
  }
  return !m_Streaming;
}

unsigned int mitk::OphirPyro::GetDataFromSensor()
{
  if (m_Streaming)
  {
    std::vector<double> newEnergy;
    std::vector<double> newTimestamp;
    std::vector<int> newStatus;
    unsigned int noPackages = 0;
    try
    {
      noPackages = ophirAPI.GetData(m_DeviceHandle, &newEnergy, &newTimestamp, &newStatus);
      if (noPackages > 0)
      {
        m_PulseEnergy.insert(m_PulseEnergy.end(), newEnergy.begin(), newEnergy.end());
        for (unsigned int i=0; i<noPackages; i++)
          m_TimeStamps.push_back(std::chrono::high_resolution_clock::now().time_since_epoch().count());

        m_PulseTime.insert(m_PulseTime.end(), newTimestamp.begin(), newTimestamp.end());
        m_PulseStatus.insert(m_PulseStatus.end(), newStatus.begin(), newStatus.end());

        m_PulseEnergySaved.insert(m_PulseEnergySaved.end(), newEnergy.begin(), newEnergy.end());
        m_PulseTimeSaved.insert(m_PulseTimeSaved.end(), newTimestamp.begin(), newTimestamp.end());
        m_PulseStatusSaved.insert(m_PulseStatusSaved.end(), newStatus.begin(), newStatus.end());
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

void mitk::OphirPyro::GetDataFromSensorThread()
{
  while(m_Connected && m_Streaming)
  {
    this->GetDataFromSensor();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  return;
}

double mitk::OphirPyro::LookupCurrentPulseEnergy()
{
  if (m_Connected && !m_PulseEnergy.empty())
  {
    MITK_INFO << m_PulseEnergy.size();
    return m_PulseEnergy.back();
  }
  return 0;
}

double mitk::OphirPyro::GetClosestEnergyInmJ(long long ImageTimeStamp, double interval)
{
  if (m_PulseTime.size() == 0)
    return 0;

  long long searchTime = (ImageTimeStamp/1000000) - m_ImagePyroDelay; // conversion from ns to ms

  //MITK_INFO << "searchTime = " << searchTime;
  int foundIndex = -1;
  long long shortestDifference = 250*interval;

  // search the list for a fitting energy value time
  for (int index = 0; index < m_PulseTime.size();++index)
  {
    long long newDifference = abs(((int)m_PulseTime[index]) - searchTime);
    //MITK_INFO << "newDifference[" << index << "] = " << newDifference;
    if (newDifference < shortestDifference)
    {
      shortestDifference = newDifference;
      foundIndex = index;
      //MITK_INFO << "foundIndex = " << foundIndex;
    }
  }

  if (abs(shortestDifference) < interval)
  {
    // delete all elements before the one found
    m_PulseEnergy.erase(m_PulseEnergy.begin(), m_PulseEnergy.begin() + foundIndex);
    m_PulseTime.erase(m_PulseTime.begin(), m_PulseTime.begin() + foundIndex);
    m_PulseStatus.erase(m_PulseStatus.begin(), m_PulseStatus.begin() + foundIndex);

    // multipy with m_EnergyMultiplicator, because the Pyro gives just a fraction of the actual Laser Energy
    return (GetNextPulseEnergy()*m_EnergyMultiplicator);
  }

  //MITK_INFO << "No matching energy value for image found in interval of " << interval << "ms. sd: " << shortestDifference;
  return -1;
}

double mitk::OphirPyro::GetNextEnergyInmJ(long long ImageTimeStamp, double interval)
{
  if (m_Connected && !(m_PulseTime.size() > 0))
    return 0;

  long long searchTime = (ImageTimeStamp / 1000000) - m_ImagePyroDelay; // conversion from ns to ms

  if (abs(searchTime - m_PulseTime.front()) < interval)
  {
    return (GetNextPulseEnergy()*m_EnergyMultiplicator); // multipy with m_EnergyMultiplicator, because the Pyro gives just a fraction of the actual Laser Energy
  }

  MITK_INFO << "Image aquisition and energy measurement ran out of sync";
  return -1;
}

void mitk::OphirPyro::SetSyncDelay(long long FirstImageTimeStamp)
{
  while (!m_PulseTime.size())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  m_ImagePyroDelay = (FirstImageTimeStamp / 1000000) - m_PulseTime.at(0);

  MITK_INFO << "m_ImagePyroDelay = " << m_ImagePyroDelay;
  return;
}

bool mitk::OphirPyro::IsSyncDelaySet()
{
  return (m_ImagePyroDelay != 0);
}

double mitk::OphirPyro::GetNextPulseEnergy()
{
  if (m_Connected && m_PulseEnergy.size()>=1)
  {
    double out = m_PulseEnergy.front();
    m_PulseEnergy.erase(m_PulseEnergy.begin());
    m_PulseTime.erase(m_PulseTime.begin());
    m_PulseStatus.erase(m_PulseStatus.begin());
    return out;
  }
  return 0;
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
    char* m_SerialNumber;
    try {
      MITK_INFO << "Scanning for Ophir connection";
      m_SerialNumber = ophirAPI.ScanUSB();
    }
    catch (...)
    {
      MITK_INFO << "Scanning failed, trying again in 2 seconds...";
      std::this_thread::sleep_for(std::chrono::seconds(2));
      MITK_INFO << "Scanning for Ophir connection";
      m_SerialNumber = ophirAPI.ScanUSB();
    }

    if (m_SerialNumber != 0)
    {
      try {
        MITK_INFO << "Opening Ophir connection";
        m_DeviceHandle = ophirAPI.OpenDevice(m_SerialNumber);
      }
      catch (...)
      {
        MITK_INFO << "Ophir connection failed, trying again in 2 seconds...";
        std::this_thread::sleep_for(std::chrono::seconds(2));
        MITK_INFO << "Opening Ophir connection";
        m_DeviceHandle = ophirAPI.OpenDevice(m_SerialNumber);
      }
      
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