#ifndef MITKCYLINDER_H_HEADER_INCLUDED
#define MITKCYLINDER_H_HEADER_INCLUDED

#include "mitkBoundingObject.h"
//#include "mitkVector.h"

namespace mitk {

//##Documentation
//## @brief Data class containing an ellipsoid
//## @ingroup Geometry
class Cylinder : public BoundingObject
{
public:
  mitkClassMacro(Cylinder, BoundingObject);  
  itkNewMacro(Self);
  
  virtual void UpdateOutputInformation();
  
  virtual bool IsInside(ITKPoint3D p);
  
  //void SetRequestedRegionToLargestPossibleRegion();
  //bool RequestedRegionIsOutsideOfTheBufferedRegion();
  //virtual bool VerifyRequestedRegion();
  //void SetRequestedRegion(itk::DataObject *data);

protected:
  Cylinder();
  virtual ~Cylinder();
};
  
  
}
#endif /* MITKCYLINDER_H_HEADER_INCLUDED */

