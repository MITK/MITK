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

bool mitk::FileWriter::CanWriteDataType( DataNode* )
{
  //TODO #345 check for writing permission
  return false;
}

std::string mitk::FileWriter::GetWritenMIMEType()
{
  return "";
}

void mitk::FileWriter::SetInput( DataNode* )
{
}


std::string mitk::FileWriter::GetFileExtension()
{
  return "";
}

std::string mitk::FileWriter::GetPossibleFileExtensionsAsString()
{
  std::vector<std::string> possibleFileExtensions = this->GetPossibleFileExtensions();
  std::stringstream stream;
  for (unsigned int i=0; i<possibleFileExtensions.size()-1; i++)
  {
    stream<<"*"<<possibleFileExtensions.at(i)<<" ";
  }
  stream<<"*"<<possibleFileExtensions.at(possibleFileExtensions.size()-1);
  return stream.str();
}

bool mitk::FileWriter::IsExtensionValid(std::string extension)
{
  std::vector<std::string> possibleFileExtensions = this->GetPossibleFileExtensions();
  for (unsigned int i=0; i<possibleFileExtensions.size(); i++)
  {
    if (strcmp(extension.c_str(),possibleFileExtensions.at(i).c_str())==0)
      return true;
  }
  return false;
}


mitk::FileWriter::FileWriter()
{
}

mitk::FileWriter::~FileWriter()
{
}

