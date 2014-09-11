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

#ifndef MITKBSPLINEREGISTRATION_H
#define MITKBSPLINEREGISTRATION_H



#include "mitkRegistrationBase.h"
#include "MitkDeformableRegistrationExports.h"
#include "mitkImageAccessByItk.h"
#include <mitkOptimizerParameters.h>
//#include <mitkMetricParameters.h>
#include "mitkRigidRegistrationObserver.h"

namespace mitk
{

  /*!
  \brief This class performes a b-spline registration between two images.

  \ingroup DeformableRegistration

  \author Thomas van Bruggen
  */


  class MITK_DEFORMABLEREGISTRATION_EXPORT BSplineRegistration : public RegistrationBase
  {

  public:


    mitkClassMacro(BSplineRegistration, RegistrationBase);

    /*!
    * \brief Method for creation through the object factory.
    */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)


    /*!
    * \brief Sets the number of iterations which will be performed during the registration process.
    */
    void SetNumberOfIterations(int iterations);


    /*!
    * \brief Sets whether the result should be saved or not.
    */
    void SetSaveResult(bool saveResult);

    /*!
    * \brief Sets the filename for the resulting deformed image.
    */
    void SetResultFileName(const char* resultName);


    /*!
    * \brief Starts the B-Spline registration.
    */
    virtual void GenerateData()
    {
      if (this->GetInput())
      {
        AccessByItk(this->GetInput(), GenerateData2);
      }
    }

    /*!
    \brief Set the optimizer parameters
    */


    void SetOptimizerParameters(mitk::OptimizerParameters::Pointer optimizerParameters)
    {
      m_OptimizerParameters = optimizerParameters;
    }

   /* void SetMetricParameters(mitk::MetricParameters::Pointer metricParameters)
    {
      m_MetricParameters = metricParameters;
    }*/


   /* void SetParameters(mitk::OptimizerParameters::Pointer optimizerParameters, mitk::MetricParameters::Pointer metricParameters)
    {
      m_OptimizerParameters = optimizerParameters;
      m_MetricParameters = metricParameters;
    }*/

    itkSetMacro(NumberOfGridPoints, int);
    itkSetMacro(SaveDeformationField, bool);
    itkSetMacro(UpdateInputImage, bool);
    itkSetMacro(DeformationFileName, std::string);
    itkSetMacro(Metric, int);
    itkSetMacro(MatchHistograms, bool);



  protected:

    /*!
    * \brief Default constructor
    */
    BSplineRegistration();

    /*!
    * \brief Default destructor
    */
    virtual ~BSplineRegistration();

    /*!
    * \brief Template class to perform the demons registration with any kind of image. Called by GenerateData().
    */
    template < typename TPixel, unsigned int VImageDimension >
      void GenerateData2( const itk::Image<TPixel, VImageDimension>* itkImage1);

    int m_Iterations;
    const char* m_ResultName;
    bool m_SaveResult;

    mitk::OptimizerParameters::Pointer m_OptimizerParameters;
    //mitk::MetricParameters::Pointer m_MetricParameters;

    int m_NumberOfGridPoints;
    bool m_SaveDeformationField;
    bool m_UpdateInputImage;
    std::string m_DeformationFileName;

    bool m_MatchHistograms;

    int m_Metric;

    RigidRegistrationObserver::Pointer m_Observer;

  };
}

#endif // MITKDEMONSREGISTRATION_H
