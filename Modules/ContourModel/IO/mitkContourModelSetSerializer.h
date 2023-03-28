/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelSetSerializer_h
#define mitkContourModelSetSerializer_h

#include <MitkContourModelExports.h>
#include <mitkBaseDataSerializer.h>

namespace mitk
{
  class MITKCONTOURMODEL_EXPORT ContourModelSetSerializer : public BaseDataSerializer
  {
  public:
    mitkClassMacro(ContourModelSetSerializer, BaseDataSerializer);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      std::string Serialize() override;

  protected:
    ContourModelSetSerializer();
    ~ContourModelSetSerializer() override;
  };
}

#endif
