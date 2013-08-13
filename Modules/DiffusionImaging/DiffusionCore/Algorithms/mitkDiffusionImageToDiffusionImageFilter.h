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

#include "mitkImageToImageFilter.h"
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
    : public ImageToImageFilter
{
public:
  /** typedefs for compatibility */
  typedef mitk::DiffusionImage< DiffusionPixelType >  DiffusionImageType;
  typedef typename DiffusionImageType::Pointer        DiffusionImagePointerType;

  mitkClassMacro( DiffusionImageToDiffusionImageFilter<DiffusionPixelType>,
                  ImageToImageFilter )

  itkNewMacro(Self)

protected:
  DiffusionImageToDiffusionImageFilter();
  virtual ~DiffusionImageToDiffusionImageFilter() {}

};

} // end namespace mitk

#include "mitkDiffusionImageToDiffusionImageFilter.cpp"

#endif // MITKDIFFUSIONIMAGETODIFFUSIONIMAGEFILTER_H
