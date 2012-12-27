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


#ifndef MITKCOREDATANODEREADER_H
#define MITKCOREDATANODEREADER_H

#include <mitkIDataNodeReader.h>

namespace mitk {

class CoreDataNodeReader : public itk::LightObject, public mitk::IDataNodeReader
{
public:

  itkNewMacro(CoreDataNodeReader)

  int Read(const std::string& fileName, mitk::DataStorage& storage);
};

}

#endif // MITKCOREDATANODEREADER_H
