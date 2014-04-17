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

#ifndef mitkPlanarFiguresSubclassesSerializer_h_included
#define mitkPlanarFiguresSubclassesSerializer_h_included

#include "mitkPlanarFigureSerializer.h"

#define MITK_REGISTER_PF_SUB_SERIALIZER(classname) \
  \
  namespace mitk \
{ \
  \
class MitkPlanarFigure_EXPORT classname ## Serializer : public PlanarFigureSerializer \
{ \
public: \
  \
  mitkClassMacro( classname ## Serializer, PlanarFigureSerializer ) \
  itkFactorylessNewMacro(Self) \
  itkCloneMacro(Self) \
  \
protected: \
  \
  classname ## Serializer () {} \
  virtual ~classname ## Serializer () {} \
}; \
  \
} \
  \
  MITK_REGISTER_SERIALIZER( classname ## Serializer );

MITK_REGISTER_PF_SUB_SERIALIZER(PlanarAngle);
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarCircle);
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarCross);
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarFourPointAngle);
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarLine);
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarPolygon);
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarSubdivisionPolygon);
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarRectangle);
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarEllipse);
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarDoubleEllipse);

#endif
