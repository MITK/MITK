#ifndef MITKPROPERTIES_H_HEADER_INCLUDED
#define MITKPROPERTIES_H_HEADER_INCLUDED

#include "mitkGenericProperty.h"

namespace mitk {
   typedef GenericProperty<int> IntegerProperty;
   template class GenericProperty<int>;
}  // namespace mitk
#endif /* MITKPROPERTIES_H_HEADER_INCLUDED */
