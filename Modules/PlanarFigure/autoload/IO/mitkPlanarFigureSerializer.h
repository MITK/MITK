/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarFigureSerializer_h
#define mitkPlanarFigureSerializer_h

#include "mitkBaseDataSerializer.h"

namespace mitk
{
  /**
    \brief Serializes mitk::Surface for mitk::SceneIO
  */
  class PlanarFigureSerializer : public BaseDataSerializer
  {
  public:
    mitkClassMacro(PlanarFigureSerializer, BaseDataSerializer);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self) std::string Serialize() override;

  protected:
    PlanarFigureSerializer();
    ~PlanarFigureSerializer() override;
  };
} // namespace
#endif
