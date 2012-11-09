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
      mitkClassMacro(USImageToUSImageFilter, mitk::ImageToImageFilter);
      itkNewMacro(Self);

      // ---- OVERRIDDEN INHERITED METHODS ---- //
      virtual void SetInput (const mitk::USImage *image);
      virtual void SetInput (unsigned int index, const mitk::USImage *image);

      virtual mitk::USImage::Pointer GetOutput(unsigned int idx);
      virtual mitk::USImage::Pointer GetInput (unsigned int idx);

    protected:
      USImageToUSImageFilter();
      virtual ~USImageToUSImageFilter();



    };
} // namespace mitk
#endif
