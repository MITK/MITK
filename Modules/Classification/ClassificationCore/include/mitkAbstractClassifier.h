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


#ifndef mitkAbstractClassifier_h
#define mitkAbstractClassifier_h

#include <MitkClassificationCoreExports.h>

#include <mitkBaseData.h>

// Eigen
#include <Eigen/Dense>

// STD Includes

// MITK includes
#include <mitkConfigurationHolder.h>

namespace mitk
{
  class MITKCLASSIFICATIONCORE_EXPORT AbstractClassifier : public BaseData
  {
  public:
    ///
    /// @brief MatrixType
    ///
    typedef Eigen::MatrixXd EigenMatrixXdType;

    ///
    /// @brief VectorType
    ///
    typedef Eigen::VectorXd EigenVectorXdType;

    ///
    /// @brief Build a forest of trees from the training set (X, y).
    /// @param X, The training input samples. Matrix of shape = [n_samples, n_features]
    /// @param Y, The target values (class labels in classification, real numbers in regression). Array of shape = [n_samples]
    ///
    virtual void Fit(const EigenMatrixXdType &X, const EigenVectorXdType &Y) = 0;

    ///
    /// @brief Predict class for X.
    /// @param X, The input samples.
    /// @return The predicted classes. Y array of shape = [n_samples]
    ///
    virtual EigenVectorXdType Predict(const EigenMatrixXdType &X) = 0;

    ///
    /// @brief SetConfiguration, handing over classifier custom configurations
    /// @param conf
    /// @deprecated Use base data propertylist
    ///
    DEPRECATED(void SetConfiguration(const ConfigurationHolder& conf));

    ///
    /// @brief Configuration
    /// @return none-const refernce to the custom configuration
    /// @deprecated Use base data propertylist
    ///
    DEPRECATED(ConfigurationHolder& Configuration());

    ///
    /// @brief GetConfigurationCopy
    /// @return Create and return a deep copy of the configuration
    /// @deprecated Use base data propertylist
    ///
    DEPRECATED(ConfigurationHolder GetConfigurationCopy());

    ///
    /// @brief SetPointWiseWeight
    /// @param W, The pointwise weights. W array of shape = [n_samples]
    ///
    void SetPointWiseWeight(const EigenVectorXdType& W);

    ///
    /// @brief GetPointWiseWeightCopy
    /// @return Create and return a copy of W
    ///
    EigenVectorXdType GetPointWiseWeightCopy();

    ///
    /// @brief UsePointWiseWeight
    /// @param toggle weighting on/off
    ///
    void UsePointWiseWeight(bool value);

    ///
    /// @brief IsUsingPointWiseWeight
    /// @return true if pointewise weighting is enabled.
    ///
    bool IsUsingPointWiseWeight();

    ///
    /// @brief SupportsPointWiseWeight
    /// @return True if the classifier supports pointwise weighting else false
    ///
    virtual bool SupportsPointWiseWeight();

  protected:
    EigenVectorXdType& PointWeight();

  private:
    ///
    /// @brief m_Config
    /// @deprecated Use base data propertylist
    ///
    DEPRECATED(ConfigurationHolder m_Config);

    EigenVectorXdType m_PointWiseWeight;
    bool m_UsePointWiseWeight;

  };
}

#endif //mitkAbstractClassifier_h
