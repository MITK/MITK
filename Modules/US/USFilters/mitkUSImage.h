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


#ifndef MITKUSIMAGE_H_HEADER_INCLUDED_
#define MITKUSIMAGE_H_HEADER_INCLUDED_

#include <MitkUSExports.h>
#include <mitkCommon.h>
#include <mitkImage.h>
#include "mitkUSImageMetadata.h"

namespace mitk {

    /**Documentation
    * \brief This specialization of mitk::Image only appends necessary Metadata to an MITK image. Otherwise it can safely be treated it's mother class.
    * \ingroup US
    */
  class MitkUS_EXPORT USImage : public mitk::Image
    {
    public:
      mitkClassMacro(USImage, mitk::Image);
      itkNewMacro(Self);

      itkGetMacro(Metadata, mitk::USImageMetadata::Pointer);   
      itkSetMacro(Metadata, mitk::USImageMetadata::Pointer);

    protected:
      USImage();
      virtual ~USImage();
      USImageMetadata::Pointer m_Metadata; 
      



    };
} // namespace mitk
#endif