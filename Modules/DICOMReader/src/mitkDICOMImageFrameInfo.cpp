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

#include "mitkDICOMImageFrameInfo.h"

mitk::DICOMImageFrameInfo
::DICOMImageFrameInfo(const std::string& filename, unsigned int frameNo)
:Filename(filename)
,FrameNo(frameNo)
{
}

bool
mitk::DICOMImageFrameInfo
::operator==(const DICOMImageFrameInfo& other) const
{
  return
     this->Filename == other.Filename
  && this->FrameNo == other.FrameNo;

}
