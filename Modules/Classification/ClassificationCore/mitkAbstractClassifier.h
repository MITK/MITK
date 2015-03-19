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

//#include <mitkBaseData.h>

// Eigen
#include <Eigen/Dense>

// STD Includes

// MITK
#include <mitkConfigurationHolder.h>

namespace mitk
{
  class MITKCLASSIFICATIONCORE_EXPORT AbstractClassifier // : public BaseData
  {
  public:
    typedef Eigen::MatrixXd MatrixType;
    typedef Eigen::VectorXd VectorType;

    virtual void Train(const MatrixType &X, const VectorType &Y) = 0;
    virtual VectorType Predict(const MatrixType &X) = 0;

    void SetConfiguration(const ConfigurationHolder& value);
    ConfigurationHolder& Configuration();
    ConfigurationHolder GetConfiguration();

    void SetPointWiseWeight(const VectorType& w);
    VectorType GetPointWiseWeight();

    void UsePointWiseWeight(bool value);
    bool IsUsingPointWiseWeight();

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