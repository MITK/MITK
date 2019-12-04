/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarFiguresSubclassesSerializer_h_included
#define mitkPlanarFiguresSubclassesSerializer_h_included

#include "mitkPlanarFigureSerializer.h"

#define MITK_REGISTER_PF_SUB_SERIALIZER(classname)                                                                     \
                                                                                                                       \
  namespace mitk                                                                                                       \
  \
{                                                                                                                 \
    \
class classname##Serializer : public PlanarFigureSerializer\
{                                                        \
      \
public :                                                                                                               \
                                                                                                                       \
        mitkClassMacro(classname##Serializer, PlanarFigureSerializer) itkFactorylessNewMacro(Self) itkCloneMacro(Self) \
      \
protected :                                                                                                            \
                                                                                                                       \
        classname##Serializer(){} virtual ~classname##Serializer(){}                                                   \
    \
};                                                                                                                     \
  \
}                                                                                                                 \
                                                                                                                       \
  MITK_REGISTER_SERIALIZER(classname##Serializer);

MITK_REGISTER_PF_SUB_SERIALIZER(PlanarAngle)
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarCircle)
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarCross)
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarFourPointAngle)
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarLine)
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarPolygon)
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarRectangle)
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarEllipse)
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarDoubleEllipse)
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarBezierCurve)
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarSubdivisionPolygon)

#endif
