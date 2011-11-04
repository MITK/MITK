#ifndef _mitkLegacyAdaptors_h
#define _mitkLegacyAdaptors_h_

#include "mitkCommon.h"
#include "LegacyAdaptorsExports.h"

#include <mitkIpPic.h>
#include "mitkImage.h"

namespace mitk
{
  /**
   \brief Constructs a legacy mitkIpPicDescriptor from mitk::Image

   Meant to support legacy code, which was base on mitkIpPicDescriptor types. Please remind that such code should be migrated towards ITK/VTK solutions.
 */
  //LegacyAdaptors_EXPORT DEPRECATED(mitkIpPicDescriptor* CastToIpPicDescriptor(mitk::Image::Pointer, mitkIpPicDescriptor* picDesc));
  LegacyAdaptors_EXPORT mitkIpPicDescriptor* CastToIpPicDescriptor(mitk::Image::Pointer, mitkIpPicDescriptor* picDesc);

  /**
    \brief Constructs a legacy mitkIpPicDescriptor form mitk::ImageDataItem
  */
  //LegacyAdaptors_EXPORT DEPRECATED(mitkIpPicDescriptor* CastToIpPicDescriptor(itk::SmartPointer<mitk::ImageDataItem>, mitkIpPicDescriptor *picDesc ));
  LegacyAdaptors_EXPORT mitkIpPicDescriptor* CastToIpPicDescriptor(itk::SmartPointer<mitk::ImageDataItem>, mitkIpPicDescriptor *picDesc );

  /**
    \brief Constructs an ImageDescriptor from legacy mitkIpPicDescriptor

    Performs the oposite cast direction to \sa CastToIpPicDescriptor
  */
  LegacyAdaptors_EXPORT DEPRECATED(mitk::ImageDescriptor::Pointer CastToImageDescriptor(mitkIpPicDescriptor* desc));

  /**
    \brief Constructs a legacy type information from mitk::PixelType

    Meant to support legacy mitkIpPicDescriptor using an own type description, different to the mitk::PixelType
  */
  LegacyAdaptors_EXPORT DEPRECATED(mitkIpPicType_t CastToIpPicType( const std::type_info& intype ) );

  LegacyAdaptors_EXPORT DEPRECATED(PixelType CastToPixelType( mitkIpPicType_t pictype, size_t bpe ) );

} // end namespace mitk

#endif
