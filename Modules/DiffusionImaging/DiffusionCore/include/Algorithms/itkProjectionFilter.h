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

#ifndef ITKPROJECTIONFILTER_H_
#define ITKPROJECTIONFILTER_H_

#include "itkObject.h"
#include "itkImage.h"
#include "mitkImage.h"
#include "mitkTbssImage.h"

namespace itk
{


/**
  * \brief Projection part of the TBSS pipeline
  *
  * This class performs the projection step of the TBSS pipeline
  * (see Smith et al., 2009. http://dx.doi.org/10.1016/j.neuroimage.2006.02.024 )
  * As input it takes a binary skeleton, a distance map, and a vector image containing the image gradients
  * that are typically provided by the itkSkeletonizationFilter and the itkDistanceMapFilter.
  * Furthermore it requires a 4d dataset with patient data registered to the same space as the mean FA image.
  * This 4D dataset is typically created by the TBSS pipeline (see http://fsl.fmrib.ox.ac.uk/fsl/fsl4.0/tbss/index),
  * in which case it is often named all_FA.nii.gz
  */


class ProjectionFilter : public Object
{

public:

  typedef itk::Image<float, 3> RealImageType;

  typedef itk::CovariantVector<int,3> VectorType;

  typedef itk::Image<VectorType, 3> VectorImageType;

  typedef itk::Image<char,3> CharImageType;

  typedef itk::Image<float, 4> Float4DImageType;
  typedef itk::Image<float, 3> FloatImageType;



public:

  /** */
  typedef ProjectionFilter Self;

  /** Superclass */
  typedef Object Superclass;

  /** Smart Pointer */
  typedef SmartPointer<Self> Pointer;

  /** Smart Pointer */
  typedef SmartPointer<const Self> ConstPointer;

  /** */
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)


  /** \brief Does the actual projection */
  void Project();


  /** \brief Set the distance map
   *
   * Sets the distance map that decodes for every voxel the distance to the nearest point on the skeleton.
   */
  itkSetMacro(DistanceMap, RealImageType::Pointer)


  /** \brief Set the directions
   *
   * Sets the direction calculated by the TBSS skeletonization algorithm in itkSkeletonizationFilter.
   */
  itkSetMacro(Directions, VectorImageType::Pointer)


  /** \brief Set the binary skeleton
   *
   * Sets the binary skeleton that defines on which voxels must be projected.
   */
  itkSetMacro(Skeleton, CharImageType::Pointer)


  /** \brief Set the mask defining tubular structures on the skeleton
   *
   * Sets a binary mask that defines wich part of the white matter skeleton are tubular instead of sheet like.
   * This is important because the a different projection method is used for sheet like structues and
   * tubular structures.
   */
  itkSetMacro(Tube, CharImageType::Pointer)


  /** \brief Set a 4D image containing the 3D registered FA maps of all study subjects. */
  itkSetMacro(AllFA, Float4DImageType::Pointer)


  /** \brief Returns a 4D image containing the skeleton projections of all subjects */
  itkGetMacro(Projections, Float4DImageType::Pointer)


protected:

  /** Constructor */
  ProjectionFilter();

  /** Destructor */
  ~ProjectionFilter() override;

  RealImageType::Pointer m_DistanceMap;

  VectorImageType::Pointer m_Directions;

  CharImageType::Pointer m_Skeleton;

  CharImageType::Pointer m_Tube;

  Float4DImageType::Pointer m_Projections;

  Float4DImageType::Pointer m_AllFA;

  int round(float x)
  {
    if (x>0.0) return ((int) (x+0.5));
    else       return ((int) (x-0.5));
  }

protected:



};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkProjectionFilter.txx"
#endif

#endif
