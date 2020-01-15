/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
