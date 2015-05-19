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
#include <mitkIOMimeTypes.h>

mitk::NavigationDataSetWriterCSV::NavigationDataSetWriterCSV() : AbstractFileWriter(NavigationDataSet::GetStaticNameOfClass(),
  CustomMimeType(NAVIGATIONDATASETCSV_MIMETYPE()),
  "MITK NavigationDataSet Reader (CSV)")
{}

mitk::NavigationDataSetWriterCSV::~NavigationDataSetWriterCSV()
{}

mitk::NavigationDataSetWriterCSV* mitk::NavigationDataSetWriterCSV::Clone() const
{
  return new NavigationDataSetWriterCSV(*this);
}

void mitk::NavigationDataSetWriterCSV::Write()
{
  std::ostream* out = GetOutputStream();
  mitk::NavigationDataSet::ConstPointer data = dynamic_cast<const NavigationDataSet*> (this->GetInput());

    //save old locale
  char * oldLocale;
  oldLocale = setlocale( LC_ALL, nullptr );

  //define own locale
  std::locale C("C");
  setlocale( LC_ALL, "C" );

  //write header
  unsigned int numberOfTools = data->GetNumberOfTools();
  for (unsigned int index = 0; index < numberOfTools; index++){ *out << "TimeStamp_Tool" << index <<
                                                                           ";Valid_Tool" << index <<
                                                                           ";X_Tool" << index <<
                                                                           ";Y_Tool" << index <<
                                                                           ";Z_Tool" << index <<
                                                                           ";QX_Tool" << index <<
                                                                           ";QY_Tool" << index <<
                                                                           ";QZ_Tool" << index <<
                                                                           ";QR_Tool" << index << ";";}
  *out << "\n";

  out->precision(15); // rounding precision because we don't want to loose data.

  //write data
  MITK_INFO << "Number of timesteps: " << data->Size();
  for (unsigned int i=0; i<data->Size(); i++)
  {
    std::vector< mitk::NavigationData::Pointer > NavigationDatasOfCurrentStep = data->GetTimeStep(i);
    for (unsigned int toolIndex = 0; toolIndex < numberOfTools; toolIndex++)
    {
      mitk::NavigationData::Pointer nd = NavigationDatasOfCurrentStep.at(toolIndex);
      *out             << nd->GetTimeStamp() << ";"
                       << nd->IsDataValid() << ";"
                       << nd->GetPosition()[0] << ";"
                       << nd->GetPosition()[1] << ";"
                       << nd->GetPosition()[2] << ";"
                       << nd->GetOrientation()[0] << ";"
                       << nd->GetOrientation()[1] << ";"
                       << nd->GetOrientation()[2] << ";"
                       << nd->GetOrientation()[3] << ";";
    }
    *out << "\n";
  }

  //switch back to old locale
  setlocale( LC_ALL, oldLocale );
}

mitk::CustomMimeType mitk::NavigationDataSetWriterCSV::NAVIGATIONDATASETCSV_MIMETYPE()
{
  mitk::CustomMimeType mimeType(IOMimeTypes::DEFAULT_BASE_NAME() + ".NavigationDataSet.csv");
  std::string category = "NavigationDataSet";
  mimeType.SetComment("NavigationDataSet (csv)");
  mimeType.SetCategory(category);
  mimeType.AddExtension("csv");
  return mimeType;
}