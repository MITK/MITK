/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkHummelProtocolEvaluation_h
#define mitkHummelProtocolEvaluation_h

#include <mitkPointSet.h>
#include <array>




namespace mitk
{

  /**Documentation
  * \brief Static methods for evaluations according to the assessment protocol
  * for EM trackers published by Hummel et al. 2005 [1].
  *
  * [1] Hummel, J. et al. - Design and application of an assessment protocol for electromagnetic tracking systems. Med Phys 32(7), July 2005
  *
  * \ingroup IGT
  */
  class HummelProtocolEvaluation

  {
  public:

    /** Distance error with description. */
    struct HummelProtocolDistanceError {double distanceError; std::string description;};
    /** Tracking volumes for evaluation.
     *  standard: The standard volume of 9 x 10 measurment points as described in [1]
     *  small: A small volume in the center 3 x 4 measurement points, for smaller field generators [2]
     *  [2] Maier-Hein, L. et al. - Standardized assessment of new electromagnetic field generators in an interventional radiology setting. Med Phys 39(6), June 2012
     */
    enum HummelProtocolMeasurementVolume { small, medium, standard };
    /** Evaluates the 5 cm distances as defined by the Hummel protocol [1,2].
     * @return Returns true if evaluation was successfull, false if not.
     * @param[out] Results Please give an empty vector. The results will be added to this vector.
     */
    static bool Evaluate5cmDistances(mitk::PointSet::Pointer p, HummelProtocolMeasurementVolume m, std::vector<HummelProtocolDistanceError> &Results);

    /** Evaluates the 15 cm distances as defined by the Hummel protocol [1,2].
    * @return Returns true if evaluation was successfull, false if not.
    * @param[out] Results Please give an empty vector. The results will be added to this vector.
    */
    static bool Evaluate15cmDistances(mitk::PointSet::Pointer p, HummelProtocolMeasurementVolume m, std::vector<HummelProtocolDistanceError> &Results);

    /** Evaluates the 30 cm distances as defined by the Hummel protocol [1,2].
    * @return Returns true if evaluation was successfull, false if not.
    * @param[out] Results Please give an empty vector. The results will be added to this vector.
    */
    static bool Evaluate30cmDistances(mitk::PointSet::Pointer p, HummelProtocolMeasurementVolume m, std::vector<HummelProtocolDistanceError> &Results);

    /** Evaluates the accumulated distances as defined by the Hummel protocol [1,2].
    * @return Returns true if evaluation was successfull, false if not.
    * @param[out] Results Please give an empty vector. The results will be added to this vector.
    */
    static bool EvaluateAccumulatedDistances(mitk::PointSet::Pointer p, HummelProtocolMeasurementVolume m, std::vector<HummelProtocolDistanceError> &Results);

    /** Computes statistics (as mean, standard deviation, quantiles, min, max, etc.) on the given values.
    *  The results are stored inside the return value.
    */
    static std::vector<HummelProtocolDistanceError> ComputeStatistics(std::vector<HummelProtocolDistanceError> values);

  protected:
    /** Converts a pointset holding all measurement points of the hummel protocol in line-by-line order
     *  to an array representing the hummel board.
     */
    static std::array<std::array<mitk::Point3D, 10> ,9> ParseMatrixStandardVolume(mitk::PointSet::Pointer p);
    //It would be really wonderfull if we could replace std::array<std::array<mitk::Point3D, 10> ,9> by  mitk::Matrix< mitk::Point3D, 9, 10 > but
    //unfortunatly this version does not compile under Linux. To be precise under Linux only matrices like this: mitk::Matriy<double, 9, 10> compile
    //even the usage of a double pointer (eg mitk::Matrix<double* , 9, 10>) does not compile. We always got an error message saying:
    //vnl_c_vector.h:42:49: error: invalid use of incomplete type ‘class vnl_numeric_traits<itk::Point<double, 3u> >’
    //Under Windows this error does not appear there everything compiles fine.

    static std::array<std::array<mitk::Point3D, 5>, 5> ParseMatrixMediumVolume(mitk::PointSet::Pointer p);

  };
} // namespace mitk

#endif
