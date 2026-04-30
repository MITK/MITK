/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkEqual_h
#define mitkEqual_h

#include <cmath>
#include <iomanip>

#include <mitkLog.h>
#include <mitkNumericConstants.h>

namespace mitk
{
  /**
   * \brief Check if the absolute difference is bigger than or equal to a given epsilon.
   *
   * \param[in] diff The difference to be checked against epsilon.
   * \param[in] epsilon The absolute difference needs to be smaller than this.
   * \return True if abs(diff) >= epsilon.
   */
  template <typename DifferenceType>
  inline bool DifferenceBiggerOrEqualEps(DifferenceType diff, mitk::ScalarType epsilon = mitk::eps)
  {
    return std::fabs(diff) >= epsilon;
  }

  /**
   * \brief Conditionally output the difference between two elements to the log.
   *
   * Outputs elem1, elem2, and eps when verbose is true and the elements are not equal.
   * Elements can e.g. be a mitk::Vector or a mitk::Point.
   *
   * \param[in] elem1 First element to be output.
   * \param[in] elem2 Second element to be output.
   * \param[in] eps The epsilon which their difference was bigger than.
   * \param[in] verbose Whether output should be generated.
   * \param[in] isEqual Only output something if the two elements are not equal.
   */
  template <typename ElementToOutput1, typename ElementToOutput2>
  inline void ConditionalOutputOfDifference(
    ElementToOutput1 elem1, ElementToOutput2 elem2, mitk::ScalarType eps, bool verbose, bool isEqual)
  {
    if (verbose && !isEqual)
    {
      MITK_INFO << typeid(ElementToOutput1).name() << " and " << typeid(ElementToOutput2).name()
                << " not equal. Lefthandside " << std::setprecision(12) << elem1 << " - Righthandside " << elem2
                << " - epsilon " << eps;
    }
  }

  /**
   * \ingroup MITKTestingAPI
   * \brief Compare two scalar values for equality within a tolerance.
   *
   * \param[in] scalar1 Scalar value to compare.
   * \param[in] scalar2 Scalar value to compare.
   * \param[in] eps Tolerance for floating point comparison.
   * \param[in] verbose Flag indicating detailed console output.
   * \return True if scalars are equal within the given tolerance.
   */
  inline bool Equal(ScalarType scalar1, ScalarType scalar2, ScalarType eps = mitk::eps, bool verbose = false)
  {
    bool isEqual(!DifferenceBiggerOrEqualEps(scalar1 - scalar2, eps));

    ConditionalOutputOfDifference(scalar1, scalar2, eps, verbose, isEqual);

    return isEqual;
  }
}

#endif
