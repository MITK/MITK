#ifndef MITCUBOID_H_HEADER_INCLUDED
#define MITCUBOID_H_HEADER_INCLUDED

#include "mitkBoundingObject.h"
//#include "mitkVector.h"

namespace mitk {

//##Documentation
//## @brief Data class containing an ellipsoid
//## @ingroup Geometry
class Cuboid : public BoundingObject
{
public:
  mitkClassMacro(Cuboid, BoundingObject);  
  itkNewMacro(Self);
  
  virtual void UpdateOutputInformation();
  
  virtual bool IsInside(ITKPoint3D p);
  
  //void SetRequestedRegionToLargestPossibleRegion();
  //bool RequestedRegionIsOutsideOfTheBufferedRegion();
  //virtual bool VerifyRequestedRegion();
  //void SetRequestedRegion(itk::DataObject *data);

protected:
  Cuboid();
  virtual ~Cuboid();
};
  
  
}
#endif /* MITCUBOID_H_HEADER_INCLUDED */

