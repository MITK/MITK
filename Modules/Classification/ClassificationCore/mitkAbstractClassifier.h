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

// MITK
#include <mitkConfigurationHolder.h>

namespace mitk
{
  class MITKCLASSIFICATIONCORE_EXPORT AbstractClassifier : public BaseData
  {
  public:
    ///
    /// @brief MatrixType
    ///
    typedef Eigen::MatrixXd MatrixType;

    ///
    /// @brief VectorType
    ///
    typedef Eigen::VectorXd VectorType;

    ///
    /// @brief Build a forest of trees from the training set (X, y).
    /// @param X, The training input samples. Matrix of shape = [n_samples, n_features]
    /// @param Y, The target values (class labels in classification, real numbers in regression). Array of shape = [n_samples]
    ///
    virtual void Fit(const MatrixType &X, const VectorType &Y) = 0;

    ///
    /// @brief Predict class for X.
    /// @param X, The input samples.
    /// @return The predicted classes. Y array of shape = [n_samples]
    ///
    virtual VectorType Predict(const MatrixType &X) = 0;

    ///
    /// @brief SetConfiguration, handing over classifier custom configurations
    /// @param conf
    ///
    void SetConfiguration(const ConfigurationHolder& conf);

    ///
    /// @brief Configuration
    /// @return none-const refernce to the custom configuration
    ///
    ConfigurationHolder& Configuration();

    ///
    /// @brief GetConfigurationCopy
    /// @return Create and return a deep copy of the configuration
    ///
    ConfigurationHolder GetConfigurationCopy();

    ///
    /// @brief SetPointWiseWeight
    /// @param W, The pointwise weights. W array of shape = [n_samples]
    ///
    void SetPointWiseWeight(const VectorType& W);

    ///
    /// @brief GetPointWiseWeightCopy
    /// @return Create and return a copy of W
    ///
    VectorType GetPointWiseWeightCopy();

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
    VectorType& PointWeight();

  private:
// this pragma ignores the following warning:
// warning C4251: 'mitk::Particle::pos' : class   'ATL::CStringT'   needs to have dll-interface to be used   by clients of class 'Particle'
#ifdef _MSC_VER
  #pragma warning(push)
  #pragma warning(disable: 4251)
#endif

    ConfigurationHolder m_Config;
    VectorType m_PointWiseWeight;
    bool m_UsePointWiseWeight;

#ifdef _MSC_VER
 #pragma warning(pop)
#endif
  };
}

#endif //mitkAbstractClassifier_h
