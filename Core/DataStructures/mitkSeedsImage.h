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


#ifndef MITKSEEDSIMAGE_H_HEADER_INCLUDED_C1C2FCD2
#define MITKSEEDSIMAGE_H_HEADER_INCLUDED_C1C2FCD2

#include "mitkImage.h"

#include <itkImage.h>
#include <itkNeighborhood.h>
#include <itkNeighborhoodIterator.h>
#include <itkGaussianSpatialFunction.h>
#include <itkBinaryBallStructuringElement.h>

namespace mitk
{

//##Documentation
//## @brief SeedsImage class for storing seeds-images
//##
//## Handles operations for drawing seeds. 
//## @ingroup Data
class MITK_CORE_EXPORT SeedsImage : public Image
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
  SeedsImage();
  virtual ~SeedsImage();

  template< typename SeedsImageType >
    itk::NeighborhoodIterator< SeedsImageType >& GetNit( SeedsImageType* image );

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

  typedef itk::GaussianSpatialFunction< int, 3 > GaussianFunction3DType;
  GaussianFunction3DType::Pointer m_GaussianFunction3D;
  typedef itk::GaussianSpatialFunction< int, 2 > GaussianFunction2DType;
  GaussianFunction2DType::Pointer m_GaussianFunction2D;

  typedef itk::BinaryBallStructuringElement< short, 3 > BallStructuringElement3DType;
  BallStructuringElement3DType m_StructuringElement3D;
  typedef itk::BinaryBallStructuringElement< short, 2 > BallStructuringElement2DType;
  BallStructuringElement2DType m_StructuringElement2D;

};

} // namespace mitk

#endif /* MITKSEEDSIMAGE_H_HEADER_INCLUDED_C1C2FCD2 */
