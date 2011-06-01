/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkDiffusionTensor3DReconstructionImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2006-03-27 17:01:06 $
  Version:   $Revision: 1.12 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkTensorDerivedMeasurementsFilter_h_
#define __itkTensorDerivedMeasurementsFilter_h_

#include "itkDiffusionTensor3D.h"
#include "itkImageToImageFilter.h"



namespace itk{
/** \class TensorDerivedMeasurementsFilter
 */

template <class TPixel>
class TensorDerivedMeasurementsFilter :
  public ImageToImageFilter<  itk::Image< itk::DiffusionTensor3D<TPixel>, 3 >, 
                              itk::Image< TPixel, 3 > >
{

public:

  enum Measure
  {
    FA,// Fractional anisotropy
    RA,// Relative anisotropy
    AD,// Axial diffusivity
    RD,// Radial diffusivity
    CA, // Clustering anisotropy 1-(λ2+λ3)/(2*λ1)
    L2,
    L3,
    MD // mean diffusivity
  };

  typedef itk::DiffusionTensor3D<TPixel>          TensorType;
  typedef itk::Image< TensorType, 3 >             TensorImageType;
  typedef itk::Image< TPixel, 3 >                 OutputImageType;

  typedef TensorDerivedMeasurementsFilter         Self;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;
  typedef ImageToImageFilter< TensorType, 
                              OutputImageType >
                                                  SuperClass;
   
   /** Method for creation through the object factory. */
  itkNewMacro(Self);  

  /** Runtime information support. */
  itkTypeMacro(TensorDerivedMeasurementsFilter, ImageToImageFilter);
 
  itkSetMacro(Measure, Measure);


protected:
  TensorDerivedMeasurementsFilter();
  ~TensorDerivedMeasurementsFilter() {};
  //void PrintSelf(std::ostream& os, Indent indent) const;
  
  Measure m_Measure; 

  void GenerateData();

}; // end class

} // end namespace

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTensorDerivedMeasurementsFilter.txx"
#endif

#endif //__itkTensorDerivedMeasurements_h_

