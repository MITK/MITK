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

/** @file mitkItkPictureWrite.h */

#ifndef MITKITKPICTUREWRITE_H
#define MITKITKPICTUREWRITE_H

#include <mitkCommon.h>
#include <itkImage.h>

/**
 * @brief ITK-Like method to be called for writing an single-component image using the AccessByItk Macros
 *
 * @param itkImage an image with single-component pixel type
 *
 * @deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
 */
template < typename TPixel, unsigned int VImageDimension >
DEPRECATED( void _mitkItkPictureWrite(itk::Image< TPixel, VImageDimension >* itkImage, const std::string& fileName) );

/**
 * @brief ITK-Like method to be called for writing an image
 *
 * @param itkImage an Image with single-component or composite pixel type
 *
 * @deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
 */
template < typename TPixel, unsigned int VImageDimension >
DEPRECATED( void _mitkItkPictureWriteComposite(itk::Image< TPixel, VImageDimension >* itkImage, const std::string& fileName) );

#endif /* MITKITKPICTUREWRITE_H */
