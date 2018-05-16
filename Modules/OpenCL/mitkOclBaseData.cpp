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

#include "mitkOclBaseData.h"

mitk::OclBaseData::OclBaseData()
: m_BufferSize ( 0 ), m_GPUBuffer( nullptr ), m_CPUBuffer( nullptr )
{
}

mitk::OclBaseData::~OclBaseData()
{
  if (m_GPUBuffer) clReleaseMemObject( m_GPUBuffer );
  if (m_CPUBuffer) free( m_CPUBuffer );
}

cl_mem mitk::OclBaseData::GetGPUBuffer()
{
  return this->m_GPUBuffer;
}
