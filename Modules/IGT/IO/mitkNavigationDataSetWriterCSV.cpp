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

#include "mitkNavigationDataSetWriterCSV.h"
#include <fstream>

void mitk::NavigationDataSetWriterCSV::Write (std::string path, mitk::NavigationDataSet::Pointer data)
{
  MITK_INFO << "Writing navigation data set to file: " << path;
  std::ofstream stream;
  stream.open (path.c_str(), std::ios_base::trunc);

  // Pass to Stream Handler
  Write(&stream, data);
  stream.close();
}

void mitk::NavigationDataSetWriterCSV::Write (std::ostream* stream, mitk::NavigationDataSet::Pointer data)
{
  //save old locale
  char * oldLocale;
  oldLocale = setlocale( LC_ALL, 0 );

  //define own locale
  std::locale C("C");
  setlocale( LC_ALL, "C" );

  //write header
  int numberOfTools = data->GetNumberOfTools();
  for (unsigned int index = 0; index < numberOfTools; index++){ *stream << "TimeStamp_Tool" << index <<
                                                                           ";Valid_Tool" << index <<
                                                                           ";X_Tool" << index <<
                                                                           ";Y_Tool" << index <<
                                                                           ";Z_Tool" << index <<
                                                                           ";QX_Tool" << index <<
                                                                           ";QY_Tool" << index <<
                                                                           ";QZ_Tool" << index <<
                                                                           ";QR_Tool" << index << ";";}
  *stream << "\n";

  stream->precision(15); // rounding precision because we don't want to loose data.

  //write data
  MITK_INFO << "Number of timesteps: " << data->Size();
  for (int i=0; i<data->Size(); i++)
  {
    std::vector< mitk::NavigationData::Pointer > NavigationDatasOfCurrentStep = data->GetTimeStep(i);
    for (int toolIndex = 0; toolIndex < numberOfTools; toolIndex++)
    {
      mitk::NavigationData::Pointer nd = NavigationDatasOfCurrentStep.at(toolIndex);
      *stream          << nd->GetTimeStamp() << ";"
                       << nd->IsDataValid() << ";"
                       << nd->GetPosition()[0] << ";"
                       << nd->GetPosition()[1] << ";"
                       << nd->GetPosition()[2] << ";"
                       << nd->GetOrientation()[0] << ";"
                       << nd->GetOrientation()[1] << ";"
                       << nd->GetOrientation()[2] << ";"
                       << nd->GetOrientation()[3] << ";";
    }
    *stream << "\n";
  }

  //switch back to old locale
  setlocale( LC_ALL, oldLocale );
}

mitk::NavigationDataSetWriterCSV::NavigationDataSetWriterCSV()
{}

mitk::NavigationDataSetWriterCSV::~NavigationDataSetWriterCSV()
{}
