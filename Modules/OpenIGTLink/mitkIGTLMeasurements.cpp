/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIGTLMeasurements.h"
#include <chrono>

//Microservices
#include "usServiceReference.h"
#include "usModuleContext.h"
#include "usServiceEvent.h"
#include "mitkServiceInterface.h"
#include "usGetModuleContext.h"
#include <iostream>
#include <fstream>

mitk::IGTLMeasurements::IGTLMeasurements()
{
}

mitk::IGTLMeasurements* mitk::IGTLMeasurements::GetInstance()
{
  static us::ServiceReference<mitk::IGTLMeasurements> serviceRef;
  static us::ModuleContext* context = us::GetModuleContext();
  if (!serviceRef)
  {
    // This is either the first time GetInstance() was called,
    // or a mitk::IGTLMeasurements instance has not yet been registered.
    serviceRef = context->GetServiceReference<mitk::IGTLMeasurements>();
  }
  if (serviceRef)
  {
    // We have a valid service reference. It always points to the service
    // with the lowest id (usually the one which was registered first).
    // This still might return a null pointer, if all mitk::IGTLMeasurements
    // instances have been unregistered (during unloading of the library,
    // for example).
    return context->GetService(serviceRef);
  }
  else
  {
    // No mitk::IGTLMeasurements instance was registered yet.
    return nullptr;
  }
}

mitk::IGTLMeasurements::~IGTLMeasurements()
{
}

void mitk::IGTLMeasurements::AddMeasurement(unsigned int measurementPoint, unsigned int index, long long timestamp)
{
  if (timestamp == 0) { timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count(); }
  if (m_IsStarted)
  {
    m_MeasurementPoints[measurementPoint].push_back(TimeStampIndexPair(timestamp, index));
  }
}

bool mitk::IGTLMeasurements::ExportData(std::string filename)
{
  //open file
  std::ostream* out = new std::ofstream(filename.c_str());

  //save old locale
  char * oldLocale;
  oldLocale = setlocale(LC_ALL, nullptr);

  //define own locale
  std::locale C("C");
  setlocale(LC_ALL, "C");

  //write header
  unsigned int numberOfMeasurementPoints = (unsigned int)m_MeasurementPoints.size();
  *out << numberOfMeasurementPoints << "\n";

  if (numberOfMeasurementPoints == 0)
  {
    delete out;
    return false;
  }

  out->precision(15); // rounding precision because we don't want to loose data.

  //for each entry of the map
  for (auto entry : m_MeasurementPoints)
  {
    *out << entry.second.size() << ";";
    *out << entry.first << ";";
    for (TimeStampIndexPair timestampIndexPair : entry.second)
    {
      *out << (timestampIndexPair.first) << ";";
      *out << (timestampIndexPair.second) << ";";
    }
    *out << "\n";
  }

  out->flush();
  delete out;
  //switch back to old locale
  setlocale(LC_ALL, oldLocale);

  return true;
}

void mitk::IGTLMeasurements::Reset()
{
  m_MeasurementPoints.clear();
}

void mitk::IGTLMeasurements::SetStarted(bool started)
{
  m_IsStarted = started;
}
