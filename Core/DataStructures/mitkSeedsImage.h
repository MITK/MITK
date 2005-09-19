/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#ifndef MITKSEEDSIMAGE_H_HEADER_INCLUDED_C1C2FCD2
#define MITKSEEDSIMAGE_H_HEADER_INCLUDED_C1C2FCD2

#include <mitkImage.h>
#include <itkImage.h>

namespace mitk {

class SeedsImage : public Image
{

public:
  
  mitkClassMacro(SeedsImage, Image);
  itkNewMacro(Self);

  virtual void ExecuteOperation(Operation* operation);

protected:
  template < typename SeedsImageType >
  void AddSeedPoint(SeedsImageType * itkImage);

  template < typename SeedsImageType >
  void PointInterpolation(SeedsImageType * itkImage);


protected:
	mitk::Point3D point;
	mitk::Point3D last_point;
  mitk::Vector3D spacing;
  int orig_size[3];
  int m_DrawState;
  int m_Radius;
  float delta_x, delta_y, delta_z;
	float sphere_distance;
};
} // namespace mitk

#endif /* MITKSEEDSIMAGE_H_HEADER_INCLUDED_C1C2FCD2 */
