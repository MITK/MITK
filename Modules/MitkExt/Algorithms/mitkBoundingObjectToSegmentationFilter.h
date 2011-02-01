/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 27918 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/
#ifndef MITKBOUNDINGOBJECTTOSEGMENTATIONFILTER_H
#define MITKBOUNDINGOBJECTTOSEGMENTATIONFILTER_H

#include <mitkImageToImageFilter.h>
#include <mitkBoundingObjectGroup.h>

namespace mitk{
  class MitkExt_EXPORT BoundingObjectToSegmentationFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(BoundingObjectToSegmentationFilter, ImageToImageFilter);
    itkNewMacro(BoundingObjectToSegmentationFilter);

    void SetBoundingObject(mitk::BoundingObject::Pointer boundingObject);
  protected:
    BoundingObjectToSegmentationFilter();
    virtual ~BoundingObjectToSegmentationFilter();

    virtual void GenerateData();

    mitk::BoundingObjectGroup::Pointer m_boundingObjectGroup;

  };//class
}//namespace
#endif