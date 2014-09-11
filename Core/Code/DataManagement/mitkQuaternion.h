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

#ifndef MITKQUATERNION_H_
#define MITKQUATERNION_H_


#include <vnl/vnl_quaternion.h>
#include "mitkNumericConstants.h"

namespace mitk {

typedef vnl_quaternion<ScalarType> Quaternion;

}

#endif /* MITKQUATERNION_H_ */
