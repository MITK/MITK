#ifndef MITKPICHELPER_H_HEADER_INCLUDED_C1F4DAB4
#define MITKPICHELPER_H_HEADER_INCLUDED_C1F4DAB4

#include "mitkCommon.h"
#include "mitkVector.h"
#include <ipPic/ipPic.h>

namespace mitk {

class SlicedGeometry3D;

//##ModelId=3D7B424502D7
//##Documentation
//## @brief Internal class for managing references on sub-images
//## @ingroup Data
class PicHelper
{
public:
  static bool GetSpacing(ipPicDescriptor* pic, Vector3D & spacing);

  static bool SetSpacing(ipPicDescriptor* pic, SlicedGeometry3D* slicedgeometry);

  static void InitializeEvenlySpaced(ipPicDescriptor* pic, unsigned int slices, SlicedGeometry3D* slicedgeometry);

  static bool SetGeometry2D(ipPicDescriptor* pic, int s, SlicedGeometry3D* slicedgeometry);
};

} // namespace mitk



#endif /* MITKPICHELPER_H_HEADER_INCLUDED_C1F4DAB4 */
