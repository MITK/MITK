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

#include "mitkDiffusionHeaderDICOMFileReader.h"

mitk::DiffusionHeaderDICOMFileReader
::DiffusionHeaderDICOMFileReader()
{

}

mitk::DiffusionHeaderDICOMFileReader
::~DiffusionHeaderDICOMFileReader()
{

}

mitk::DiffusionHeaderDICOMFileReader::DICOMHeaderListType
mitk::DiffusionHeaderDICOMFileReader
::GetHeaderInformation()
{
  if( m_HeaderInformationList.size() < 1 )
  {
    MITK_WARN << "No information retrieved yet. Call AnalyzeInputFiles first!";
  }

  return m_HeaderInformationList;
}
