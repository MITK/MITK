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

#ifndef ITKSPLITDWIMAGEFILTER_H
#define ITKSPLITDWIMAGEFILTER_H

#include "itkImageToImageFilter.h"
#include <itkVectorImage.h>

namespace itk
{

/** \class SplitDWImageFilter
 *
 * \brief Splits a DW-Image passed in as input into a 3D-t image where each volume coresponds to a
 * gradient image ( or the unweighted b0 )
 */

template< class TInputImagePixelType,
          class TOutputImagePixelType >
class SplitDWImageFilter :
    public ImageToImageFilter< VectorImage<TInputImagePixelType, 3>,
    Image< TOutputImagePixelType, 4 > >
{
public:

  typedef SplitDWImageFilter  Self;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;

  typedef TInputImagePixelType                    InputPixelType;
  typedef TOutputImagePixelType                   OutputPixelType;

  typedef ImageToImageFilter< VectorImage<TInputImagePixelType, 3>,
      Image< TOutputImagePixelType, 4 > >         Superclass;

  /** typedefs from superclass */
  typedef typename Superclass::InputImageType     InputImageType;
  //typedef typename Superclass::OutputImageType    OutputImageType;
  typedef Image< TOutputImagePixelType, 4 >       OutputImageType;
  typedef typename OutputImageType::RegionType    OutputImageRegionType;


  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Runtime information support. */
  itkTypeMacro(SplitDWImageFilter, ImageToImageFilter);

protected:

  SplitDWImageFilter();
  virtual ~SplitDWImageFilter(){};

  void GenerateData();

  /** The dimension of the output does not match the dimension of the input
    hence we need to re-implement the CopyInformation method to avoid
    executing the default implementation which tries to copy the input information to the
    output
  */
  virtual void CopyInformation( const DataObject *data);

  /** Override of the ProcessObject::GenerateOutputInformation() because of different
    dimensionality of the input and the output
  */
  virtual void GenerateOutputInformation();
};

} //end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkSplitDWImageFilter.txx"
#endif

#endif // ITKSPLITDWIMAGEFILTER_H
