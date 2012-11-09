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


#ifndef MITKSCENEDATANODEREADER_H
#define MITKSCENEDATANODEREADER_H

#include <mitkIDataNodeReader.h>

namespace mitk {

class SceneDataNodeReader : public itk::LightObject, public mitk::IDataNodeReader
{

public:

  itkNewMacro(SceneDataNodeReader)

  int Read(const std::string& fileName, mitk::DataStorage& storage);
};

}

#endif // MITKSCENEDATANODEREADER_H
