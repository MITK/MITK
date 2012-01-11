#ifndef _mitkLegacyAdaptors_h
#define _mitkLegacyAdaptors_h_

#include "mitkCommon.h"
#include "LegacyAdaptorsExports.h"

#include <mitkIpPic.h>
#include "mitkImage.h"

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
  LegacyAdaptors_EXPORT mitkIpPicDescriptor* CastToIpPicDescriptor(mitk::Image::Pointer, mitkIpPicDescriptor* picDesc);

  /**
    \brief Constructs a legacy mitkIpPicDescriptor form mitk::ImageDataItem
  */
  LegacyAdaptors_EXPORT mitkIpPicDescriptor* CastToIpPicDescriptor(itk::SmartPointer<mitk::ImageDataItem>, mitkIpPicDescriptor *picDesc );

  /**
    \brief Constructs an ImageDescriptor from legacy mitkIpPicDescriptor

    Performs the oposite cast direction to \sa CastToIpPicDescriptor
  */
  LegacyAdaptors_EXPORT DEPRECATED(mitk::ImageDescriptor::Pointer CastToImageDescriptor(mitkIpPicDescriptor* desc));

  /**
    \brief Constructs a legacy type information from mitk::PixelType

    The IpPicDescriptor uses own notations for different pixel types. This casting is needed e.g. by the CastToIpPicDescriptor method.
  */
  LegacyAdaptors_EXPORT DEPRECATED(mitkIpPicType_t CastToIpPicType( const std::type_info& intype ) );

  /**
    \brief Returns a mitk::PixelType object corresponding to given mitkIpPicType_t

    The method needs the mitkIpPicType_t and the bits per element (bpe) information to be able to create the correct corresponding PixelType
    \sa PixelType
  */
  LegacyAdaptors_EXPORT DEPRECATED(PixelType CastToPixelType( mitkIpPicType_t pictype, size_t bpe ) );

} // end namespace mitk

#endif
