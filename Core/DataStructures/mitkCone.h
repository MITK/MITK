#ifndef MITKCONE_H_HEADER_INCLUDED
#define MITKCONE_H_HEADER_INCLUDED

#include "mitkBoundingObject.h"

namespace mitk {

//##Documentation
//## @brief Data class containing an cylinder
//## @ingroup Data
class Cone : public BoundingObject
{
public:
  mitkClassMacro(Cone, BoundingObject);  
  itkNewMacro(Self);
  
  virtual bool IsInside(ITKPoint3D p);
  //virtual void UpdateOutputInformation();
protected:
  Cone();
  virtual ~Cone();
};

}
#endif /* MITKCONE_H_HEADER_INCLUDED */

