/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkLookupTables.h"

template class mitk::GenericLookupTable<bool>;
template class mitk::GenericLookupTable<float>;
template class mitk::GenericLookupTable<int>;
template class mitk::GenericLookupTable<std::string>;

mitkSpecializeGenericLookupTableOperator(mitk::BoolLookupTable);
mitkSpecializeGenericLookupTableOperator(mitk::FloatLookupTable);
mitkSpecializeGenericLookupTableOperator(mitk::IntLookupTable);
mitkSpecializeGenericLookupTableOperator(mitk::StringLookupTable);
