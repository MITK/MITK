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

#ifndef mitkGeneralizedLinearModel_h
#define mitkGeneralizedLinearModel_h

#include <MitkCLImportanceWeightingExports.h>

#include <vnl/vnl_matrix.h>
#include <vnl/vnl_vector.h>

namespace mitk
{
  /**
  * \brief Generalized Linear Model that allows linear models for non-gaussian data
  *
  * Generalized linear models are an extension of standard linear models that allow
  * a different apperance of the data. This is for example usefull to calculate
  * Logistic regressions.
  */
  class MITKCLIMPORTANCEWEIGHTING_EXPORT GeneralizedLinearModel
  {
  public:
    /**
    * \brief Initialization of the GLM. The parameters needs to be passed at the beginning.
    *
    * Constructor for a GLM. During the creation process the glm model parameter
    * are guessed.
    *
    * @param xData The input data matrix.
    * @param yData The output data matrix. The values of y must meet the requirements of the link and distribution.
    * @param addConstantColumn Default=True. If True an constant value is added to each row allowing a constant factor in the model.
    */
    GeneralizedLinearModel (const vnl_matrix<double> &xData, const vnl_vector<double> &yData, bool addConstantColumn=true);

    /**
    * \brief Predicts the value corresponding to the given vector.
    *
    * From the learned data a guess is given depending on the provided input vector. The
    * value depend on the b-values of the learned model as well as on the chosen link and
    * distribution.
    *
    * No input validation is done. The data and the learned model might not match!
    *
    * @paaram c Column for which the data is guessed.
    */
    double Predict(const vnl_vector<double> &c);

    /**
    * \brief Predicts the value corresponding to the given matrix.
    *
    * From the learned data a guess is given depending on the provided input matrix. The
    * value depend on the b-values of the learned model as well as on the chosen link and
    * distribution.
    *
    * No input validation is done. The data and the learned model might not match!
    *
    * @paaram x Matrix for which the data is guessed.
    */
    vnl_vector<double> Predict(const vnl_matrix<double> &x);

    /**
    * \brief Estimation of the exponential factor for a given function
    *
    * Gives the exponential part of a link function. Only suitable for log-it models. This
    * is especially usefull for calculating the weights for transfer learning since it
    * is equal to the weights.
    *
    */
    vnl_vector<double> ExpMu(const vnl_matrix<double> &x);

    /**
    * \brief Returns the b-Vector for the estimation
    */
    vnl_vector<double> B();

  private:

    // Estimates the rank of the matrix and creates a permutation vector so
    // that the most important columns are first. Depends on a QR-algorithm.
    void EstimatePermutation(const vnl_matrix<double> &xData);

    vnl_vector<unsigned int>    m_Permutation;        // Holds a permutation matrix which is used during calculation of B
    vnl_vector<double>          m_B;                  // B-Values. Linear componentn of the model.
    bool                        m_AddConstantColumn;  // If true, a constant value is added to each row
//    int                         m_Rank;               // The estimated input rank of the matrix.
  };
}

#endif //mitkGeneralizedLInearModel_h
