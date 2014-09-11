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
#ifndef _mitkLegacyAdaptors_h_
#define _mitkLegacyAdaptors_h_

#include "mitkCommon.h"
#include <MitkLegacyAdaptorsExports.h>

#include <mitkIpPic.h>
#include "mitkImage.h"
#include "mitkImageWriteAccessor.h"

/** \file mitkLegacyAdaptors.h
  *
  * The LegacyAdaptors module is intended to provide an interface to deprecated types/objects/methods as part of backward compatibility
  * Currently the module offers methods for casting an Image or an ImageDataItem respectively to a mitkIpPicDescriptor object since several Algorigthms
  * in MITK Modules still use the deprecated IpPic format and algorithms.
  *
  * \warning The IpPic-related classes are no more accessible through the Core Module. The dependencies needs to be set directly for the affected module. Furthermore,
  * if you want to cast information between mitk::Image and mitkIpPicDescriptor, also the dependency to the LegacyAdaptors module needs to be set.
  */

namespace mitk
{
  /**
   \brief Constructs a legacy mitkIpPicDescriptor from mitk::Image

   Meant to support legacy code, which was base on mitkIpPicDescriptor types. Please remind that such code should be migrated towards ITK/VTK solutions.
 */
  MitkLegacyAdaptors_EXPORT mitkIpPicDescriptor* CastToIpPicDescriptor(mitk::Image::Pointer, mitk::ImageWriteAccessor*, mitkIpPicDescriptor* picDesc);

  /**
    \brief Constructs a legacy mitkIpPicDescriptor from mitk::ImageDataItem

    \warning The returned IpPicDescriptor is only referencing the memory block with the data managed by the given ImageDataItem parameter.
    Simply calling ipPicFree( desc ) will delete the data and so will the ImageDataItem try when it get deleted. Simplest way to avoid the duplicate
    deletion attempt is to set the desc->data manually to NULL before calling the ipPicFree() on the descriptor
  */
  MitkLegacyAdaptors_EXPORT mitkIpPicDescriptor* CastToIpPicDescriptor(itk::SmartPointer<mitk::ImageDataItem>, mitk::ImageWriteAccessor*, mitkIpPicDescriptor *picDesc );

 /**
   \brief Constructs a deprecated legacy mitkIpPicDescriptor from mitk::Image

   Meant to support legacy code, which was base on mitkIpPicDescriptor types. Please remind that such code should be migrated towards ITK/VTK solutions.
   \deprecatedSince{2012_09} Please use image accessors instead! An image accessor (ImageWriteAccessor) can be commited to CastToIpPicDescriptor.
 */
  MitkLegacyAdaptors_EXPORT DEPRECATED(mitkIpPicDescriptor* CastToIpPicDescriptor(mitk::Image::Pointer, mitkIpPicDescriptor* picDesc));

  /**
    \brief Constructs a deprecated legacy mitkIpPicDescriptor from mitk::ImageDataItem

    \warning The returned IpPicDescriptor is only referencing the memory block with the data managed by the given ImageDataItem parameter.
    Simply calling ipPicFree( desc ) will delete the data and so will the ImageDataItem try when it get deleted. Simplest way to avoid the duplicate
    deletion attempt is to set the desc->data manually to NULL before calling the ipPicFree() on the descriptor
   \deprecatedSince{2012_09} Please use image accessors instead! An image accessor (ImageWriteAccessor) can be commited to CastToIpPicDescriptor.
  */
  MitkLegacyAdaptors_EXPORT DEPRECATED(mitkIpPicDescriptor* CastToIpPicDescriptor(itk::SmartPointer<mitk::ImageDataItem>, mitkIpPicDescriptor *picDesc));


  /**
    \brief Constructs an ImageDescriptor from legacy mitkIpPicDescriptor

    Performs the oposite cast direction to \sa CastToIpPicDescriptor
  */
  MitkLegacyAdaptors_EXPORT DEPRECATED(mitk::ImageDescriptor::Pointer CastToImageDescriptor(mitkIpPicDescriptor* desc));

  /**
    \brief Constructs a legacy type information from mitk::PixelType

    The IpPicDescriptor uses own notations for different pixel types. This casting is needed e.g. by the CastToIpPicDescriptor method.
  */
  MitkLegacyAdaptors_EXPORT DEPRECATED(mitkIpPicType_t CastToIpPicType( int componentType ) );

  /**
    \brief Returns a mitk::PixelType object corresponding to given mitkIpPicType_t

    The method needs the mitkIpPicType_t and the bits per element (bpe) information to be able to create the correct corresponding PixelType
    \sa PixelType
  */
  MitkLegacyAdaptors_EXPORT DEPRECATED(PixelType CastToPixelType( mitkIpPicType_t pictype, size_t bpe ) );

} // end namespace mitk

#endif
