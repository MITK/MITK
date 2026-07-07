/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkDoseValueType_h
#define mitkDoseValueType_h

namespace mitk
{

/**
 * \brief Represents absolute dose values in Gray (Gy).
 *
 * Type alias for double, used throughout the RT module to clearly indicate
 * that a value represents an absolute radiation dose.
 *
 * \sa DoseValueRel
 */
typedef double DoseValueAbs;

/**
 * \brief Represents relative dose values as a fraction (e.g. 0.95 = 95%).
 *
 * Type alias for double, used throughout the RT module to clearly indicate
 * that a value represents a relative radiation dose with respect to a reference dose.
 *
 * \sa DoseValueAbs
 */
typedef double DoseValueRel;


} // namespace mitk

#endif
