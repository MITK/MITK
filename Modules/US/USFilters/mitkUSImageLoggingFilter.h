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

#ifndef MITKUSImageLoggingFiler_H_HEADER_INCLUDED_
#define MITKUSImageLoggingFiler_H_HEADER_INCLUDED_

// MITK
#include <MitkUSExports.h>
#include <mitkImageToImageFilter.h>


namespace mitk {
  /**
  * \brief TODO
  *
  * \ingroup US
  */
  class MitkUS_EXPORT USImageLoggingFilter : public mitk::ImageToImageFilter
  {
  public:

    mitkClassMacro(USImageLoggingFilter, mitk::ImageToImageFilter);

  protected:
    USImageLoggingFilter();
    virtual ~USImageLoggingFilter();
  };
} // namespace mitk
#endif /* MITKUSImageSource_H_HEADER_INCLUDED_ */
