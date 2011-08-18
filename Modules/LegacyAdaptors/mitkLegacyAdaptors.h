#ifndef _mitkLegacyAdaptors_h
#define _mitkLegacyAdaptors_h_

#include "mitkCommon.h"

#include <mitkIpPic.h>
#include "mitkImage.h"

namespace mitk
{
  /**
   \brief Constructs a legacy mitkIpPicDescriptor from mitk::Image

   Meant to support legacy code, which was base on mitkIpPicDescriptor types. Please remind that such code should be migrated towards ITK/VTK solutions.
 */
  DEPRECATED(mitkIpPicDescriptor* CastToIpPicDescriptor(mitk::Image::Pointer));

  /**
    \brief Constructs an ImageDescriptor from legacy mitkIpPicDescriptor

    Performs the oposite cast direction to \sa CastToIpPicDescriptor
  */
  DEPRECATED(mitk::ImageDescriptor::Pointer CastToImageDescriptor(mitkIpPicDescriptor* desc));

  /**
    \brief Constructs a legacy type information from mitk::PixelType

    Meant to support legacy mitkIpPicDescriptor using an own type description, different to the mitk::PixelType
  */
  DEPRECATED(mitkIpPicType_t CastToIpPicType( const mitk::PixelType&) );

} // end namespace mitk

#endif
