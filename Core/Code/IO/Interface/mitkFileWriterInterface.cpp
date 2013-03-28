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

#include "mitkFileWriterInterface.h"

bool mitk::FileWriterInterface::CanWriteDataType( DataNode* )
{
  //TODO #345 check for writing permission
  return false;
}


void mitk::FileWriterInterface::SetInput( DataNode* )
{
}


std::string mitk::FileWriterInterface::GetFileExtension()
{
  return "";
}



mitk::FileWriterInterface::FileWriterInterface() :
m_CanWriteToMemory(false),
m_WriteToMemory(false)
{
}

mitk::FileWriterInterface::~FileWriterInterface()
{
}
