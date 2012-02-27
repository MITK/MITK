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
