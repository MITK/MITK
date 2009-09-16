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
    
    //void CopyConstruct(mitk::Image::Pointer img);

  protected:
    QBallImage();
    virtual ~QBallImage();
  };

} // namespace mitk

#endif /* __mitkQBallImage__h */
