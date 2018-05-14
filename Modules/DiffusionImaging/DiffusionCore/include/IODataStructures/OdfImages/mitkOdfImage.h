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


#ifndef __mitkOdfImage__h
#define __mitkOdfImage__h

#include "mitkImage.h"
#include "itkVectorImage.h"
#include "mitkImageVtkAccessor.h"

#include <MitkDiffusionCoreExports.h>

// generate by n-fold subdivisions of an icosahedron
// 1 - 12
// 2 - 42
// 3 - 92
// 4 - 162
// 5 - 252
// 6 - 362
// 7 - 492
// 8 - 642
// 9 - 812
// 10 - 1002
#define ODF_SAMPLING_SIZE 252

namespace mitk
{

  /**
  * \brief this class encapsulates orientation distribution images
  */
  class MITKDIFFUSIONCORE_EXPORT OdfImage : public Image
  {

  public:

    typedef float ScalarPixelType;
    typedef itk::Vector< ScalarPixelType, ODF_SAMPLING_SIZE > PixelType;
    typedef itk::Image< itk::Vector< ScalarPixelType, ODF_SAMPLING_SIZE >, 3 > ItkOdfImageType;

    mitkClassMacro( OdfImage, Image )
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual const vtkImageData* GetNonRgbVtkImageData(int t = 0, int n = 0) const;
    virtual vtkImageData* GetNonRgbVtkImageData(int t = 0, int n = 0);

    const vtkImageData* GetVtkImageData(int t = 0, int n = 0) const override;
    vtkImageData* GetVtkImageData(int t = 0, int n = 0) override;

    virtual void ConstructRgbImage() const;

  protected:
    OdfImage();
    ~OdfImage() override;

    mutable mitk::Image::Pointer m_RgbImage;

  };

} // namespace mitk

#endif /* __mitkOdfImage__h */
