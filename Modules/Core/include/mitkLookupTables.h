/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKLOOKUPTABLES_H_HEADER_INCLUDED
#define MITKLOOKUPTABLES_H_HEADER_INCLUDED

#include "mitkGenericLookupTable.h"

namespace mitk
{
  /**Documentation
  * \brief specializations of GenericLookupTable
  *
  * This file contains specializations of mitk::GenericLookupTable
  * for bool, float, int and std::string lookuptables
  * \warning you have to call the mitkSpecializeGenericLookupTableOperator macro
  * in mitkLookupTables.cpp with each specialization to add an ostream << operator
  * for that lookuptable specialization.
  */
  mitkSpecializeGenericLookupTable(BoolLookupTable, bool);
  mitkSpecializeGenericLookupTable(FloatLookupTable, float);
  mitkSpecializeGenericLookupTable(IntLookupTable, int);
  mitkSpecializeGenericLookupTable(StringLookupTable, std::string);
} // namespace mitk

#endif /* MITKLOOKUPTABLES_H_HEADER_INCLUDED*/
