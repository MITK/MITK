/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkImageToImageFilter.h,v $
Language:  C++
Date:      $Date: 2009-04-29 16:56:31 +0200 (Wed, 29 Apr 2009) $
Version:   $Revision: 17023 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKBSPLINEREGISTRATION_H
#define MITKBSPLINEREGISTRATION_H



#include "mitkRegistrationBase.h"
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

  
  class MITKEXT_CORE_EXPORT BSplineRegistration : public RegistrationBase
  {

  public:
    

    mitkClassMacro(BSplineRegistration, RegistrationBase);

    /*!  
    * \brief Method for creation through the object factory. 
    */
    itkNewMacro(Self);
   

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
      void GenerateData2( itk::Image<TPixel, VImageDimension>* itkImage1);

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
