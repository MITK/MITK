/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMTagCache.h"

mitk::DICOMTagCache::DICOMTagCache()
:itk::Object()
{
}

mitk::DICOMTagCache::DICOMTagCache( const DICOMTagCache&)
:itk::Object()
{
}

mitk::DICOMTagCache::~DICOMTagCache()
{
}

void mitk::DICOMTagCache::SetInputFiles(const StringList& filenames)
{
  m_InputFilenames = filenames;
  this->Modified();
}
