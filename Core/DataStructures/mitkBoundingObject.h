#ifndef BOUNDINGOBJECT_H_HEADER_INCLUDED
#define BOUNDINGOBJECT_H_HEADER_INCLUDED

//#include "mitkBaseData.h"
#include <mitkSurfaceData.h>
#include "mitkVector.h"

namespace mitk {

//##Documentation
//## @brief Data class containing an ellipsoid
//## @ingroup Geometry
class BoundingObject : public mitk::SurfaceData     //BaseData
{
public:
  mitkClassMacro(BoundingObject, mitk::SurfaceData);  
//  itkNewMacro(Self);

  virtual void UpdateOutputInformation() = 0;
  void SetRequestedRegionToLargestPossibleRegion();
  bool RequestedRegionIsOutsideOfTheBufferedRegion();
  virtual bool VerifyRequestedRegion();
  void SetRequestedRegion(itk::DataObject *data);

  virtual bool IsInside(mitk::ITKPoint3D p)=0;  
protected:
  BoundingObject();
  virtual ~BoundingObject();
};
  
  
}
#endif /* BOUNDINGOBJECT_H_HEADER_INCLUDED */

