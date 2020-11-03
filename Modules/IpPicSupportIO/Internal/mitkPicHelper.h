/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKPICHELPER_H_HEADER_INCLUDED_C1F4DAB4
#define MITKPICHELPER_H_HEADER_INCLUDED_C1F4DAB4

#include "mitkVector.h"
#include <mitkCommon.h>
#include <mitkIpPic.h>

namespace mitk
{
  class SlicedGeometry3D;

  //##Documentation
  //## @brief Internal class for managing references on sub-images
  class PicHelper
  {
  public:
    static const char *GetNameOfClass() { return "PicHelper"; }
    static bool GetSpacing(const mitkIpPicDescriptor *pic, Vector3D &spacing);

    static bool SetSpacing(const mitkIpPicDescriptor *pic, SlicedGeometry3D *slicedgeometry);

    static bool GetTimeSpacing(const mitkIpPicDescriptor *pic, float &timeSpacing);

    static void InitializeEvenlySpaced(const mitkIpPicDescriptor *pic,
                                       unsigned int slices,
                                       SlicedGeometry3D *slicedgeometry);

    static bool SetPlaneGeometry(const mitkIpPicDescriptor *pic, int s, SlicedGeometry3D *slicedgeometry);
    /**
* \deprecatedSince{2014_10} Please use SetPlaneGeometry
*/
    DEPRECATED(static bool SetGeometry2D(const mitkIpPicDescriptor *pic, int s, SlicedGeometry3D *slicedgeometry))
    {
      return SetPlaneGeometry(pic, s, slicedgeometry);
    };
  };

} // namespace mitk

#endif /* MITKPICHELPER_H_HEADER_INCLUDED_C1F4DAB4 */
