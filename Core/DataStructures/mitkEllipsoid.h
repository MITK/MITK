#ifndef MITKELLIPSEDATA_H_HEADER_INCLUDED
#define MITKELLIPSEDATA_H_HEADER_INCLUDED

#include "mitkBoundingObject.h"

namespace mitk {

//##Documentation
//## @brief Data class containing an ellipsoid
//## @ingroup Data
class Ellipsoid : public BoundingObject
{
public:
  mitkClassMacro(Ellipsoid, BoundingObject);  
  itkNewMacro(Self);

  virtual bool IsInside(ITKPoint3D p);

protected:
  Ellipsoid();
  virtual ~Ellipsoid();
};

}
#endif /* MITKELLIPSEDATA_H_HEADER_INCLUDED */
