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

//##Documentation
//## @brief SeedsImage class for storing seeds-images
//##
//## Handles operations for drawing seeds. 
//## @ingroup Data
class SeedsImage : public Image
{

public:
  
  mitkClassMacro(SeedsImage, Image);
  itkNewMacro(Self);

  itkGetMacro(LastPoint, mitk::Point3D);

  void Initialize(); 

  /// handles operations
  virtual void ExecuteOperation(Operation* operation);

  void ClearBuffer();


protected:

  void CreateBrush();

  /// sets a sphere of seeds around the point
  template < typename SeedsImageType >
  void AddSeedPoint(SeedsImageType * itkImage);

  /// interpolates a tube of seeds between two points
  template < typename SeedsImageType >
  void PointInterpolation(SeedsImageType * itkImage);

  /// interpolates a tube of seeds between two points
  template < typename SeedsImageType >
  void ClearBuffer(SeedsImageType * itkImage);


protected:
  mitk::Point3D m_Point;
  mitk::Point3D m_LastPoint;
  mitk::Vector3D m_Spacing;
  int orig_size[3];
  int m_DrawState;
  int m_Radius;
  float delta_x, delta_y, delta_z;
  float sphere_distance;

  typedef itk::Image<float,3> MaskImageType;
  MaskImageType::Pointer m_Brush;
};
} // namespace mitk

#endif /* MITKSEEDSIMAGE_H_HEADER_INCLUDED_C1C2FCD2 */
