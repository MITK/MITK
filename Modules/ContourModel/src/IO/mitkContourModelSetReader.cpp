/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkContourModelSetReader.h"
#include "mitkContourModelReader.h"
#include <fstream>
#include <iostream>
#include <mitkCustomMimeType.h>
#include <mitkLocaleSwitch.h>

mitk::ContourModelSetReader::ContourModelSetReader(const mitk::ContourModelSetReader &other)
  : mitk::AbstractFileReader(other)
{
}

mitk::ContourModelSetReader::ContourModelSetReader() : AbstractFileReader()
{
  std::string category = "ContourModelSet File";
  CustomMimeType customMimeType;
  customMimeType.SetCategory(category);
  customMimeType.AddExtension("cnt_set");

  this->SetDescription(category);
  this->SetMimeType(customMimeType);

  m_ServiceReg = this->RegisterService();
}

mitk::ContourModelSetReader::~ContourModelSetReader()
{
}

std::vector<itk::SmartPointer<mitk::BaseData>> mitk::ContourModelSetReader::DoRead()
{
  std::vector<itk::SmartPointer<mitk::BaseData>> result;
  std::vector<itk::SmartPointer<mitk::BaseData>> internalResult;

  std::string location = GetInputLocation();

  // Switch the current locale to "C"
  LocaleSwitch localeSwitch("C");

  try
  {
    mitk::ContourModelSet::Pointer contourSet = mitk::ContourModelSet::New();

    mitk::ContourModelReader reader;
    reader.SetInput(location);
    internalResult = reader.Read();

    for (unsigned int i = 0; i < internalResult.size(); ++i)
    {
      contourSet->AddContourModel(dynamic_cast<mitk::ContourModel *>(internalResult.at(i).GetPointer()));
    }
    result.push_back(dynamic_cast<mitk::BaseData *>(contourSet.GetPointer()));
  }
  catch (...)
  {
    MITK_ERROR << "Cannot read contourModel.";
  }

  return result;
}

mitk::ContourModelSetReader *mitk::ContourModelSetReader::Clone() const
{
  return new ContourModelSetReader(*this);
}
