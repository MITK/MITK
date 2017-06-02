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

#include "mitkBaseDataSource.h"
#include "mitkBaseData.h"

mitk::BaseDataSource::BaseDataSource()
{
}

mitk::BaseDataSource::~BaseDataSource()
{
}

void mitk::BaseDataSource::GraftOutput(OutputType *graft)
{
  this->GraftNthOutput(0, graft);
}

void mitk::BaseDataSource::GraftOutput(const DataObjectIdentifierType &key, OutputType *graft)
{
  if (!graft)
  {
    itkExceptionMacro(<< "Requested to graft output that is a nullptr pointer");
  }

  // we use the process object method since all our output may not be
  // of the same type
  itk::DataObject *output = this->ProcessObject::GetOutput(key);

  // Call GraftImage to copy meta-information, regions, and the pixel container
  output->Graft(graft);
}

void mitk::BaseDataSource::GraftNthOutput(unsigned int idx, OutputType *graft)
{
  if (idx >= this->GetNumberOfIndexedOutputs())
  {
    itkExceptionMacro(<< "Requested to graft output " << idx << " but this filter only has "
                      << this->GetNumberOfIndexedOutputs()
                      << " indexed Outputs.");
  }
  this->GraftOutput(this->MakeNameFromOutputIndex(idx), graft);
}

bool mitk::BaseDataSource::Updating() const
{
  return this->m_Updating;
}

mitkBaseDataSourceGetOutputDefinitions(mitk::BaseDataSource)
