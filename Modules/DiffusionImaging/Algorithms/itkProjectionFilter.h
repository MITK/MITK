/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-03-19 18:26:32 +0100 (Mo, 19 Mär 2007) $
Version:   $Revision: 9819 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef ITKPROJECTIONFILTER_H_
#define ITKPROJECTIONFILTER_H_

#include "itkObject.h"
#include "itkImage.h"
#include "mitkImage.h"
#include "mitkTbssImage.h"

namespace itk
{
class ProjectionFilter : public Object
{
	/*!
	\brief itkProjectionFilter 
	
	\brief Performs the Projection Step of the TBSS algorithm from Smith et al. 2006.
	
	
	*/

public:	

  typedef itk::Image<float, 3> RealImageType;

  typedef itk::CovariantVector<int,3> VectorType;

  typedef itk::Image<VectorType, 3> VectorImageType;

  typedef itk::Image<char,3> CharImageType;

  typedef itk::Image<float, 4> Float4DImageType;

 // typedef itk::VectorImageType<float, 3> SubjectVolumesType;


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
  itkNewMacro( Self);

	/** Generate Data. The image will be divided into a number of pieces, a number of threads 
  will be spawned and Threaded GenerateData() will be called in each thread. */
  void Project();

  itkSetMacro(DistanceMap, RealImageType::Pointer);

  itkSetMacro(Directions, VectorImageType::Pointer);

  itkSetMacro(Skeleton, CharImageType::Pointer);

  itkSetMacro(Tube, CharImageType::Pointer);

  itkSetMacro(AllFA, Float4DImageType::Pointer);

  itkGetMacro(Projections, Float4DImageType::Pointer);

  itkGetMacro(Projected, RealImageType::Pointer);


 
protected:

	/** Constructor */
  ProjectionFilter();
  
  /** Destructor */
  virtual ~ProjectionFilter();

  RealImageType::Pointer m_DistanceMap;

  VectorImageType::Pointer m_Directions;

  CharImageType::Pointer m_Skeleton;

  CharImageType::Pointer m_Tube;  

  Float4DImageType::Pointer m_Projections;

  Float4DImageType::Pointer m_AllFA;
  
  RealImageType::Pointer m_Projected;

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
