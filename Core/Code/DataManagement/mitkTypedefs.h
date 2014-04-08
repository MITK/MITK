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


#include <itkIndex.h>
#include <itkContinuousIndex.h>
#include <itkVector.h>
#include <itkMatrix.h>
#include <itkTransform.h>
#include <vnl/vnl_quaternion.h>
#include <utility>

#include "mitkTypeBasics.h"

//template<class TCoordRep, unsigned int NPointDimension>  class Vector;

namespace mitk {

typedef vnl_matrix_fixed<ScalarType, 3, 3> VnlMatrix3D;

typedef vnl_vector<ScalarType> VnlVector;
typedef vnl_vector_ref<ScalarType> VnlVectorRef;

typedef itk::Index<3> Index3D;
typedef itk::ContinuousIndex<ScalarType, 3> ContinuousIndex3D;
typedef vnl_quaternion<ScalarType> Quaternion;

typedef itk::NumericTraits<mitk::ScalarType> ScalarTypeNumericTraits;

typedef std::pair<std::string, bool> FileServiceOption;

}

#endif /* MITKTYPEDEFS_H_ */
