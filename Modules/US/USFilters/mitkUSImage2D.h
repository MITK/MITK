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


#ifndef MITKUSIMAGE2D_H_HEADER_INCLUDED_
#define MITKUSIMAGE2D_H_HEADER_INCLUDED_

#include <MitkUSExports.h>
#include <mitkCommon.h>
#include <mitkImage.h>
#include <cv.h>

namespace mitk {

    /**Documentation
    * \brief TODO
    * \ingroup US
    */
  class MitkUS_EXPORT USImage2D : public itk::Object, public IplImage //TODO: doppelvererbung nötig?
    {
    public:
      mitkClassMacro(USImage2D, itk::Object);
      itkNewMacro(Self);

    protected:
      USImage2D();
      virtual ~USImage2D();

    };
} // namespace mitk
#endif