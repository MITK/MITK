#ifndef MITKCYLINDER_H_HEADER_INCLUDED
#define MITKCYLINDER_H_HEADER_INCLUDED

#include "mitkBoundingObject.h"

namespace mitk {

//##Documentation
//## @brief Data class containing an cylinder
//## @ingroup Data
class Cylinder : public BoundingObject
{
public:
  mitkClassMacro(Cylinder, BoundingObject);  
  itkNewMacro(Self);
  
  virtual mitk::ScalarType GetVolume();
  virtual bool IsInside(Point3D p);
    
protected:
  Cylinder();
  virtual ~Cylinder();
};

}
#endif /* MITKCYLINDER_H_HEADER_INCLUDED */

