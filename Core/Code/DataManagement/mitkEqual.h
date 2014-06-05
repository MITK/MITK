/*
 * mitkEqual.h
 *
 *  Created on: Apr 14, 2014
 *      Author: wirkert
 */

#ifndef MITKEQUAL_H_
#define MITKEQUAL_H_

#include <iomanip>

#include "mitkNumericConstants.h"
#include "mitkLogMacros.h"

namespace mitk {

  /**
   * Helper method to check if the difference is bigger or equal to a given epsilon
   *
   * @param diff the difference to be checked against the epsilon
   * @param the epsilon. The absolute difference needs to be smaller than this.
   * @return true if abs(diff) >= eps
   */
  template <typename DifferenceType>
  inline bool DifferenceBiggerOrEqualEps(DifferenceType diff, mitk::ScalarType epsilon = mitk::eps)
  {
    return fabs(diff) >= epsilon;
  }

  /**
   * outputs elem1, elem2 and eps in case verbose and !isEqual.
   * Elem can e.g. be a mitk::Vector or an mitk::Point.
   *
   * @param elem1 first element to be output
   * @param elem2 second
   * @param eps the epsilon which their difference was bigger than
   * @param verbose tells the function if something shall be output
   * @param isEqual function will only output something if the two elements are not equal
   */
  template <typename ElementToOutput1, typename ElementToOutput2>
  inline void ConditionalOutputOfDifference(ElementToOutput1 elem1, ElementToOutput2 elem2, mitk::ScalarType eps, bool verbose, bool isEqual)
  {
    if(verbose && !isEqual)
    {
      MITK_INFO << typeid(ElementToOutput1).name() << " and " << typeid(ElementToOutput2).name() << " not equal. Lefthandside " << std::setprecision(12) << elem1 << " - Righthandside " << elem2 << " - epsilon " << eps;
    }
  }

  /**
   * @ingroup MITKTestingAPI
   *
   * @param scalar1 Scalar value to compare.
   * @param scalar2 Scalar value to compare.
   * @param eps Tolerance for floating point comparison.
   * @param verbose Flag indicating detailed console output.
   * @return True if scalars are equal.
   */
  inline bool Equal(ScalarType scalar1, ScalarType scalar2, ScalarType eps=mitk::eps, bool verbose=false)
  {
    bool isEqual( !DifferenceBiggerOrEqualEps(scalar1-scalar2, eps));

    ConditionalOutputOfDifference(scalar1, scalar2, eps, verbose, isEqual);

    return isEqual;
  }

}



#endif /* MITKEQUAL_H_ */
