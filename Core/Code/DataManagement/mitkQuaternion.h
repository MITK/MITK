/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKTYPEDEFS_H_
#define MITKTYPEDEFS_H_


#include <vnl/vnl_quaternion.h>
#include "mitkConstants.h"

namespace mitk {

typedef vnl_quaternion<ScalarType> Quaternion;

}

#endif /* MITKTYPEDEFS_H_ */
