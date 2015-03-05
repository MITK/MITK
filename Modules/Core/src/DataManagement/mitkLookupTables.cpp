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


#include "mitkLookupTables.h"

template class mitk::GenericLookupTable<bool>;
template class mitk::GenericLookupTable<float>;
template class mitk::GenericLookupTable<int>;
template class mitk::GenericLookupTable<std::string>;


mitkSpecializeGenericLookupTableOperator(mitk::BoolLookupTable);
mitkSpecializeGenericLookupTableOperator(mitk::FloatLookupTable);
mitkSpecializeGenericLookupTableOperator(mitk::IntLookupTable);
mitkSpecializeGenericLookupTableOperator(mitk::StringLookupTable);
