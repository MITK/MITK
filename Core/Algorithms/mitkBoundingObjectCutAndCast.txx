/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKBOUNDINGOBJECTCUTANDCAST_TXX
#define MITKBOUNDINGOBJECTCUTANDCAST_TXX

#include "mitkBoundingObjectCutAndCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkImage.h"
#include "mitkBoundingObject.h"
#include "mitkGeometry3D.h"
#include "mitkImageToItk.h"
#include "mitkStatusBar.h"

#include <vtkTransform.h>

namespace mitk
{

template <typename TPixel>
BoundingObjectCutAndCast<TPixel>::BoundingObjectCutAndCast()
{

}

template <typename TPixel>
BoundingObjectCutAndCast<TPixel>::~BoundingObjectCutAndCast()
{
}

template <typename TPixel>
const std::type_info& BoundingObjectCutAndCast<TPixel>::GetOutputPixelType() 
{
  return typeid(TPixel);
}

template < typename TPixel > 
void BoundingObjectCutAndCast<TPixel>::GenerateData()
{
  mitk::Image::ConstPointer input = this->GetInput();

  if(input.IsNull())
    return;  

  if(m_BoundingObject.IsNull())
    return;

  Access3DByItk(input, CutImageFixedOutputType);

  m_TimeOfHeaderInitialization.Modified();
}

} // of namespace mitk
#endif // of MITKBOUNDINGOBJECTCUTANDCAST_TXX
