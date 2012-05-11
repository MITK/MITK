/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKUSImageToUSImageFilter_H_HEADER_INCLUDED_
#define MITKUSImageToUSImageFilter_H_HEADER_INCLUDED_

#include <MitkUSExports.h>
#include <mitkCommon.h>
#include <mitkImageToImageFilter.h>
#include "mitkUSImage.h"

namespace mitk {

    /**Documentation
    * \brief Todo
    * \ingroup US
    */
  class MitkUS_EXPORT USImageToUSImageFilter : public mitk::ImageToImageFilter
    {
    public:
      mitkClassMacro(USImageToUSImageFilter,itk::Object);
      itkNewMacro(Self);
/*
      // ---- OVERRIDDEN INHERITED METHODS ---- //
      virtual void SetInput (const mitk::USImage *image);
      virtual void SetInput (unsigned int index, const mitk::USImage *image);

      virtual mitk::USImage::Pointer GetOutput(unsigned int idx);
  */    

    protected:
      USImageToUSImageFilter();
      virtual ~USImageToUSImageFilter();


    
    };
} // namespace mitk
#endif