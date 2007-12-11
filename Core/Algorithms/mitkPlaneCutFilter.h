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


#ifndef PLANECUTFILTER_H_HEADER_INCLUDED_C1F48A22
#define PLANECUTFILTER_H_HEADER_INCLUDED_C1F48A22

#include "mitkCommon.h"
#include "mitkImageToImageFilter.h"
#include "mitkPlaneGeometry.h"
#include "itkVectorContainer.h"

namespace mitk {

//##Documentation
//## @brief Filter to cut an image with planes. Everything in 
//## the direction of the normal of the planes will be set to a 
//## specified value.
//##
//## @todo only one plane supported yet, add 4D support
//## @warning first version, not yet tested.
//## @ingroup Process
class PlaneCutFilter : public ImageToImageFilter
{
public:
  mitkClassMacro(PlaneCutFilter, ImageToImageFilter);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  typedef enum {Fill, FillInverse} CreationMode;

  typedef itk::VectorContainer<unsigned int, PlaneGeometry::ConstPointer> PlanesContainerType;

  //##Documentation
  //## @brief Set background grey level
  itkSetMacro(BackgroundLevel, float);
  itkGetMacro(BackgroundLevel, float);

  itkSetMacro(CreationMode, CreationMode);
  itkGetMacro(CreationMode, CreationMode);

  itkSetObjectMacro(Planes, PlanesContainerType);
  itkGetObjectMacro(Planes, PlanesContainerType);
protected:
  virtual void GenerateData();

  PlaneCutFilter();

  ~PlaneCutFilter();

  float m_BackgroundLevel;

  PlanesContainerType::Pointer m_Planes;

  CreationMode m_CreationMode;
};

} // namespace mitk

#endif /* PLANECUTFILTER_H_HEADER_INCLUDED_C1F48A22 */

