/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLookupTables_h
#define mitkLookupTables_h

#include <mitkGenericLookupTable.h>

namespace mitk
{
  /**
   * \brief Specializations of GenericLookupTable for common value types.
   *
   * This file contains specializations of mitk::GenericLookupTable
   * for bool, float, int, and std::string lookup tables.
   *
   * \warning You must call the mitkSpecializeGenericLookupTableOperator macro
   * in mitkLookupTables.cpp for each specialization to add an ostream << operator
   * for that lookup table specialization.
   *
   * \sa GenericLookupTable
   */
  mitkSpecializeGenericLookupTable(BoolLookupTable, bool);
  mitkSpecializeGenericLookupTable(FloatLookupTable, float);
  mitkSpecializeGenericLookupTable(IntLookupTable, int);
  mitkSpecializeGenericLookupTable(StringLookupTable, std::string);
} // namespace mitk

#endif
