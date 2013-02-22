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


#include "mitkPhilipsDicomDiffusionImageHeaderReader.h"

mitk::PhilipsDicomDiffusionImageHeaderReader::PhilipsDicomDiffusionImageHeaderReader()
{
}

mitk::PhilipsDicomDiffusionImageHeaderReader::~PhilipsDicomDiffusionImageHeaderReader()
{
}

// do the work
void mitk::PhilipsDicomDiffusionImageHeaderReader::Update()
{

  // check if there are filenames
  if(m_DicomFilenames.size())
  {
    ReadPublicTags();

    ReadPublicTags2();

    TransformGradients();

  }
}
