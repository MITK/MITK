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


#ifndef MITKPICHELPER_H_HEADER_INCLUDED_C1F4DAB4
#define MITKPICHELPER_H_HEADER_INCLUDED_C1F4DAB4

#include <IpPicSupportExports.h>
#include "mitkVector.h"
#include <mitkIpPic.h>

namespace mitk {

class SlicedGeometry3D;

//##Documentation
//## @brief Internal class for managing references on sub-images
//## @ingroup Data
class IpPicSupport_EXPORT PicHelper
{
public:
  static const char *GetNameOfClass() { return "PicHelper"; }

  static bool GetSpacing(const mitkIpPicDescriptor* pic, Vector3D & spacing);

  static bool SetSpacing(const mitkIpPicDescriptor* pic, SlicedGeometry3D* slicedgeometry);

  static bool GetTimeSpacing(const mitkIpPicDescriptor* pic, float& timeSpacing);

  static void InitializeEvenlySpaced(const mitkIpPicDescriptor* pic, unsigned int slices, SlicedGeometry3D* slicedgeometry);

  static bool SetGeometry2D(const mitkIpPicDescriptor* pic, int s, SlicedGeometry3D* slicedgeometry);
};

} // namespace mitk



#endif /* MITKPICHELPER_H_HEADER_INCLUDED_C1F4DAB4 */
