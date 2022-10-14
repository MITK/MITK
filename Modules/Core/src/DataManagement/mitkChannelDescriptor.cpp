/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkChannelDescriptor.h>

mitk::ChannelDescriptor::ChannelDescriptor(mitk::PixelType type, size_t numOfElements, bool /*allocate*/)
  : m_PixelType(type), m_Size(numOfElements), m_Data(nullptr)
{
}

mitk::ChannelDescriptor::~ChannelDescriptor()
{
}
