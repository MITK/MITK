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

#ifndef MITKCUBOIDOBJECTCUTTER_H_
#define MITKCUBOIDOBJECTCUTTER_H_

#include <mitkBoundingObjectCutter.h>
#include "MitkExtExports.h"
#include <mitkCuboid.h>

namespace mitk
{

class MitkExt_EXPORT CuboidObjectCutter : public BoundingObjectCutter
{
public:
  mitkClassMacro(CuboidObjectCutter, BoundingObjectCutter);
  itkNewMacro(Self);

  void SetBoundingObject( const Cuboid* cuboid )
  {
    BoundingObjectCutter::SetBoundingObject(cuboid);
  }

  const Cuboid* GetBoundingObject() const
  {
    return static_cast<Cuboid*>(this->m_BoundingObject.GetPointer());
  }
protected:
  template < typename TPixel, unsigned int VImageDimension, typename TOutputPixel >
  static void CutImageWithOutputTypeSelect( itk::Image< TPixel, VImageDimension >* inputItkImage, mitk::CuboidObjectCutter* cutter, int boTimeStep=0, TOutputPixel* dummy=NULL );

  template < typename TPixel, unsigned int VImageDimension >
  static void CutImage( itk::Image< TPixel, VImageDimension >* itkImage, mitk::CuboidObjectCutter* cutter, int boTimeStep );

  virtual void ComputeData(mitk::Image* input3D, int boTimeStep);

  CuboidObjectCutter();
  virtual
  ~CuboidObjectCutter();
};

}

#endif /* MITKCUBOIDOBJECTCUTTER_H_ */
