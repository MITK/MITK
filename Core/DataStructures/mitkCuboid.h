#ifndef MITCUBOID_H_HEADER_INCLUDED
#define MITCUBOID_H_HEADER_INCLUDED

#include "mitkBoundingObject.h"

namespace mitk {

//##Documentation
//## @brief Data class containing an cuboid
//## @ingroup Data
class Cuboid : public BoundingObject
{
public:
  mitkClassMacro(Cuboid, BoundingObject);  
  itkNewMacro(Self);

  virtual mitk::ScalarType GetVolume();
  virtual bool IsInside(Point3D p);

protected:
  Cuboid();
  virtual ~Cuboid();
};

}
#endif /* MITCUBOID_H_HEADER_INCLUDED */

