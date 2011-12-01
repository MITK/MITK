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

#ifndef ITKDISTANCEMAPFILTER_H_
#define ITKDISTANCEMAPFILTER_H_

#include "itkImageToImageFilter.h"
#include "itkImage.h"
#include "mitkImage.h"


namespace itk
{
template < class TInputImage, class TOutputImage >
class DistanceMapFilter : public ImageToImageFilter<TInputImage, TOutputImage>
{
	/*!
	\brief itkDistanceMapFilter 
	
	\brief Creates a distance map from a FA skeleton image.
	
	\sa itkImageToImageFilter
	
	
	\verbatim
	Last contributor: $Author: vanbrugg $
	\endverbatim
	*/

public:

	/** Typedef for input ImageType. */
  typedef TInputImage  InputImageType;

  /** Typedef for input imageType Pointer. */
  typedef typename InputImageType::Pointer InputImagePointer;

  /** Typedef for output ImageType. */
  typedef TOutputImage OutputImageType;

  /** Typedef for input imageType Pointer. */
  typedef typename OutputImageType::Pointer OutputImagePointer;

public:

	/** */
  typedef DistanceMapFilter Self;
  
  /** Superclass */
  typedef ImageToImageFilter<InputImageType, OutputImageType> Superclass;
  
  /** Smart Pointer */
  typedef SmartPointer<Self> Pointer;
  
  /** Smart Pointer */
  typedef SmartPointer<const Self> ConstPointer;

	/** */
  itkNewMacro( Self);

	/** Generate Data. The image will be divided into a number of pieces, a number of threads 
  will be spawned and Threaded GenerateData() will be called in each thread. */
  virtual void GenerateData();

 
protected:

	/** Constructor */
  DistanceMapFilter();
  
  /** Destructor */
  virtual ~DistanceMapFilter();

  
  
protected:
 	
  	
  
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDistanceMapFilter.txx"
#endif

#endif 
