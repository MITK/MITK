#ifndef MITKPROPERTIES_H_HEADER_INCLUDED
#define MITKPROPERTIES_H_HEADER_INCLUDED

#include "mitkGenericProperty.h"
#include "vecmath.h"
#include <string>

namespace mitk {

   typedef GenericProperty<bool> BoolProperty;
   template class GenericProperty<bool>;

   typedef GenericProperty<int> IntProperty;
   template class GenericProperty<int>;

   typedef GenericProperty<float> FloatProperty;
   template class GenericProperty<float>;

   typedef GenericProperty<double> DoubleProperty;
   template class GenericProperty<double>;

   typedef GenericProperty<Point3d> Point3dProperty;
   template class GenericProperty<Point3d>;

   typedef GenericProperty<Point3<int> > Point3iProperty;
   template class GenericProperty<Point3<int> >;
   
//   typedef GenericProperty<int> IntProperty;
//   template class GenericProperty<int>;
            
}  // namespace mitk
#endif /* MITKPROPERTIES_H_HEADER_INCLUDED */
