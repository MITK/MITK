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

#ifndef mitkPlanarFigureCompositeSerializer_h_included
#define mitkPlanarFigureCompositeSerializer_h_included

#include "mitkBaseDataSerializer.h"

namespace mitk
{
/**
  \brief Serializes mitk::PlanarFigureComposite for mitk::SceneIO
*/
class PlanarFigureCompositeSerializer : public BaseDataSerializer
{
  public:
    mitkClassMacro( PlanarFigureCompositeSerializer, BaseDataSerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    std::string Serialize() override;
  protected:
    PlanarFigureCompositeSerializer();
    ~PlanarFigureCompositeSerializer() override;
};
} // namespace
#endif
