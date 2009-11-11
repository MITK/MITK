/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkPlanarFiguresSubclassesSerializer_h_included
#define mitkPlanarFiguresSubclassesSerializer_h_included

#include "mitkPlanarFigureSerializer.h"

#define MITK_REGISTER_PF_SUB_SERIALIZER(classname) \
  \
  namespace mitk \
{ \
  \
class SceneSerialization_EXPORT classname ## Serializer : public PlanarFigureSerializer \
{ \
public: \
  \
  mitkClassMacro( classname ## Serializer, PlanarFigureSerializer ); \
  itkNewMacro(Self); \
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
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarLine);
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarCircle);
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarPolygon);
MITK_REGISTER_PF_SUB_SERIALIZER(PlanarFourPointAngle);

#endif
