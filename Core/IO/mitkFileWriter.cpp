/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkFileWriter.h"

bool mitk::FileWriter::CanWrite( DataTreeNode* )
{
  return false;
}

std::string mitk::FileWriter::GetWritenMIMEType()
{
  return "";
}

void mitk::FileWriter::SetInput( DataTreeNode* )
{
}

std::string mitk::FileWriter::GetFileExtension()
{
  return "";
}

mitk::FileWriter::FileWriter()
{
}

mitk::FileWriter::~FileWriter()
{
}

