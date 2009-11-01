/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2008-02-07 17:17:57 +0100 (Do, 07 Feb 2008) $
Version:   $Revision: 11989 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef __mitkQBallImage__h
#define __mitkQBallImage__h

#include "mitkImage.h"
#include "itkVectorImage.h"

#include "MitkDiffusionImagingExports.h"

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
  class MitkDiffusionImaging_EXPORT QBallImage : public Image
  {

  public:

    mitkClassMacro( QBallImage, Image );
    itkNewMacro(Self);
    
    virtual vtkImageData* GetNonRgbVtkImageData(int t = 0, int n = 0);

    virtual vtkImageData* GetVtkImageData(int t = 0, int n = 0);

    virtual void ConstructRgbImage();

  protected:
    QBallImage();
    virtual ~QBallImage();
    
    mitk::Image::Pointer m_RgbImage;

  };

} // namespace mitk

#endif /* __mitkQBallImage__h */
