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


#ifndef __mitkQBallImage__h
#define __mitkQBallImage__h

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
#define QBALL_ODFSIZE 252

namespace mitk
{

  /**
  * \brief this class encapsulates qball images
  */
  class MitkDiffusionCore_EXPORT QBallImage : public Image
  {

  public:

    mitkClassMacro( QBallImage, Image );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual const vtkImageData* GetNonRgbVtkImageData(int t = 0, int n = 0) const;
    virtual vtkImageData* GetNonRgbVtkImageData(int t = 0, int n = 0);

    virtual const vtkImageData* GetVtkImageData(int t = 0, int n = 0) const;
    virtual vtkImageData* GetVtkImageData(int t = 0, int n = 0);

    virtual void ConstructRgbImage() const;

  protected:
    QBallImage();
    virtual ~QBallImage();

    mutable mitk::Image::Pointer m_RgbImage;

  };

} // namespace mitk

#endif /* __mitkQBallImage__h */
