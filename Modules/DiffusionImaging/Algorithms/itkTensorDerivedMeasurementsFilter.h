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

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/
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

