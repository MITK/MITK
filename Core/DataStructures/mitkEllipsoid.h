#ifndef MITKELLIPSEDATA_H_HEADER_INCLUDED
#define MITKELLIPSEDATA_H_HEADER_INCLUDED

#include "mitkBoundingObject.h"
//#include "mitkVector.h"

namespace mitk {

//##Documentation
//## @brief Data class containing an ellipsoid
//## @ingroup Geometry
class Ellipsoid : public BoundingObject
{
public:
  mitkClassMacro(Ellipsoid, BoundingObject);  
  itkNewMacro(Self);
  
  virtual void UpdateOutputInformation();
  
  virtual bool IsInside(ITKPoint3D p);
  
  //void SetRequestedRegionToLargestPossibleRegion();
  //bool RequestedRegionIsOutsideOfTheBufferedRegion();
  //virtual bool VerifyRequestedRegion();
  //void SetRequestedRegion(itk::DataObject *data);

protected:
  Ellipsoid();
  virtual ~Ellipsoid();
};
  
  
}
#endif /* MITKELLIPSEDATA_H_HEADER_INCLUDED */

