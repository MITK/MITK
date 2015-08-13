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
   //get the context
   us::ModuleContext* context = us::GetModuleContext();

   std::vector<us::ServiceReference<IGTLMeasurementsImplementation>> serviceRefs =
      context->GetServiceReferences<IGTLMeasurementsImplementation>();

   //check if a service reference was found. It is also possible that several
   //services were found. This is not checked here, just the first one is taken.
   if (serviceRefs.size())
   {
      m_Measurements =
         context->GetService<mitk::IGTLMeasurementsImplementation>(serviceRefs.front());

      if (m_Measurements.IsNotNull())
      {
         m_Measurements = mitk::IGTLMeasurementsImplementation::New();
         m_Measurements->RegisterAsMicroservice();
      }
   }
   else
   {
      m_Measurements = mitk::IGTLMeasurementsImplementation::New();
      m_Measurements->RegisterAsMicroservice();
   }
}

mitk::IGTLMeasurements::~IGTLMeasurements()
{
}

void mitk::IGTLMeasurements::AddMeasurement(unsigned int measurementPoint)
{
   m_Measurements->AddMeasurement(measurementPoint);
}

bool mitk::IGTLMeasurements::ExportData(std::string filename)
{
   return m_Measurements->ExportData(filename);
}

mitk::IGTLMeasurementsImplementation::IGTLMeasurementsImplementation()
{
}

mitk::IGTLMeasurementsImplementation::~IGTLMeasurementsImplementation()
{
}

void mitk::IGTLMeasurementsImplementation::AddMeasurement(unsigned int measurementPoint)
{
   long long now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
   m_MeasurementPoints[measurementPoint].push_back(now);
}

void mitk::IGTLMeasurementsImplementation::RegisterAsMicroservice()
{
   // Get Context
   us::ModuleContext* context = us::GetModuleContext();

   // Define ServiceProps
   us::ServiceProperties props;
   m_ServiceRegistration = context->RegisterService(this, props);
}

void mitk::IGTLMeasurementsImplementation::UnRegisterMicroservice()
{
   if (m_ServiceRegistration != nullptr)
      m_ServiceRegistration.Unregister();
   m_ServiceRegistration = 0;
}



bool mitk::IGTLMeasurementsImplementation::ExportData(std::string filename)
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
   unsigned int numberOfMeasurementPoints = m_MeasurementPoints.size();
   *out << numberOfMeasurementPoints << "\n";

   if (numberOfMeasurementPoints == 0)
   {
      delete out;
      return false;
   }

   out->precision(15); // rounding precision because we don't want to loose data.

   //for each entry of the map
   for each (auto entry in m_MeasurementPoints)
   {
      *out << entry.second.size() << ";";
      for each (auto timestep in entry.second)
      {
         *out << timestep << ";";
      }
      *out << "\n";
   }

   out->flush();
   delete out;
   //switch back to old locale
   setlocale(LC_ALL, oldLocale);

   return true;
}
