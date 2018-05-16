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

#ifndef MITKCONTOURMODELSERIALIZER_H
#define MITKCONTOURMODELSERIALIZER_H

#include <MitkContourModelExports.h>
#include <mitkBaseDataSerializer.h>

namespace mitk
{
  class MITKCONTOURMODEL_EXPORT ContourModelSerializer : public BaseDataSerializer
  {
  public:
    mitkClassMacro(ContourModelSerializer, BaseDataSerializer);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      std::string Serialize() override;

  protected:
    ContourModelSerializer();
    ~ContourModelSerializer() override;
  };
}

#endif // MITKCONTOURMODELSERIALIZER_H
