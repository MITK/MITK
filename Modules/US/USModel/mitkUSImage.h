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


#ifndef MITKUSIMAGE_H_HEADER_INCLUDED_
#define MITKUSIMAGE_H_HEADER_INCLUDED_

#include <MitkUSExports.h>
#include <mitkCommon.h>
#include <mitkImage.h>
#include "mitkUSImageMetadata.h"

namespace mitk {

    /**DocumentationZ
    * \brief This specialization of mitk::Image only appends necessary Metadata to an MITK image. Otherwise it can safely be treated like it's mother class.
    *  To generate an USImage from a standard mitkImage, call the appropriate constructor USImage(image::Pointer)
    * \ingroup US
    *
    * \deprecated Deprecated since 2014-06 -> use mitk::Image instead
    */
  class MitkUS_EXPORT USImage : public mitk::Image
    {
    public:
      mitkClassMacro(USImage, mitk::Image);
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)
      /**
      * \brief this constructor creates an US Image identical to the recieved mitkImage. The Metadata are set to default.
      *  The image data is shared, so don't continue to manipulate the original image.
      * @throw mitk::Exception Throws an exception if there is a problem with access to the data while constructing the image.
      */
      mitkNewMacro1Param(Self, mitk::Image::Pointer);

      /**
      * \brief Reads out this image's Metadata set from the properties and returns a corresponding USImageMetadata object.
      */
      mitk::USImageMetadata::Pointer GetMetadata();

      /**
      * \brief Writes the information of the metadata object into the image's properties.
      */
      void SetMetadata(mitk::USImageMetadata::Pointer metadata);


    protected:
      /**
      * \brief This constructor creates an empty USImage. The Metadata are set to default.
      */
      USImage();
      /**
      * \brief this constructor creates an US Image identical to the recieved mitkImage. The Metadata are set to default.
      *  The image data is shared, so don't continue to manipulate the original image.
      */
      USImage(mitk::Image::Pointer image);
      virtual ~USImage();


    };
} // namespace mitk
#endif
