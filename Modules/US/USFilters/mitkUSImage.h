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
    * \brief This specialization of mitk::Image only appends necessary Metadata to an MITK image. Otherwise it can safely be treated like it's mother class.
    * \ingroup US
    */
  class MitkUS_EXPORT USImage : public mitk::Image
    {
    public:
      mitkClassMacro(USImage, mitk::Image);
      itkNewMacro(Self);

      /**
      * \brief reads out this image's Metadata set from the properties and returns a corresponding USImageMetadata object.
      */
      mitk::USImageMetadata::Pointer GetMetadata();

      /**
      * \brief writes the information of the metadata object into the image's properties.
      */
      void SetMetadata(mitk::USImageMetadata::Pointer metadata);


    protected:
      USImage();
      virtual ~USImage();    
      

    };
} // namespace mitk
#endif