#ifndef BOUNDINGOBJECT_H_HEADER_INCLUDED
#define BOUNDINGOBJECT_H_HEADER_INCLUDED

#include <mitkSurfaceData.h>

namespace mitk {

//##Documentation
//## @brief superclass of the different bounding objects (cylinder, cuboid,...)
//## Manages generic functions and provides an interface for IsInside()
//## calculates a generic bounding box
//## @ingroup Data
class BoundingObject : public mitk::SurfaceData     //BaseData
{
public:
  mitkClassMacro(BoundingObject, mitk::SurfaceData);  

  virtual void UpdateOutputInformation();// = 0;
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
