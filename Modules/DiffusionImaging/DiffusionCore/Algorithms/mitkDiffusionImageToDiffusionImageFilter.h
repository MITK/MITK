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

#ifndef MITKDIFFUSIONIMAGETODIFFUSIONIMAGEFILTER_H
#define MITKDIFFUSIONIMAGETODIFFUSIONIMAGEFILTER_H

#include "mitkDiffusionImageSource.h"
#include "mitkDiffusionImage.h"


namespace mitk {
/**
 * @class DiffusionImageToDiffusionImageFilter
 * @brief Base class for all DiffusionImage filters
 *
 * The class inherits the mitk::ImageToImageFilter to gain access to the filtering pipline.
 */
template <typename DiffusionPixelType>
class DiffusionImageToDiffusionImageFilter
    : public DiffusionImageSource< DiffusionPixelType >
{
public:
  /** typedefs for compatibility */
  typedef mitk::DiffusionImage< DiffusionPixelType >     InputImageType;
  typedef typename InputImageType::Pointer               InputImagePointerType;

  mitkClassMacro( DiffusionImageToDiffusionImageFilter,
                  DiffusionImageSource<DiffusionPixelType> )

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  typedef typename Superclass::OutputType                OutputImageType;
  typedef typename OutputImageType::Pointer              OutputImagePointerType;

  using itk::ProcessObject::SetInput;
  virtual void SetInput( const InputImageType* image);
  virtual void SetInput( unsigned int, const InputImageType* image);

  const InputImageType* GetInput(void);
  const InputImageType* GetInput(unsigned int);


protected:
  DiffusionImageToDiffusionImageFilter();
  virtual ~DiffusionImageToDiffusionImageFilter() {}

  virtual void PrintSelf(std::ostream &os, itk::Indent indent) const {}
  virtual void GenerateInputRequestedRegion() {}

private:
  // purposely not implemented
  void operator=(const Self&);
};

} // end namespace mitk

#include "mitkDiffusionImageToDiffusionImageFilter.cpp"

#endif // MITKDIFFUSIONIMAGETODIFFUSIONIMAGEFILTER_H
