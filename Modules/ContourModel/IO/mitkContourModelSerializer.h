/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      std::string Serialize() override;

  protected:
    ContourModelSerializer();
    ~ContourModelSerializer() override;
  };
}

#endif // MITKCONTOURMODELSERIALIZER_H
