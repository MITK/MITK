#ifndef MITKPROPERTIES_H_HEADER_INCLUDED
#define MITKPROPERTIES_H_HEADER_INCLUDED

#include "mitkGenericProperty.h"
#include "vecmath.h"

namespace mitk {
   typedef GenericProperty<int> IntProperty;
   template class GenericProperty<int>;

   typedef GenericProperty<Point3d> Point3dProperty;
   template class GenericProperty<Point3d>;

//   typedef GenericProperty<int> IntProperty;
//   template class GenericProperty<int>;
            
}  // namespace mitk
#endif /* MITKPROPERTIES_H_HEADER_INCLUDED */
