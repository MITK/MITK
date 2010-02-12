/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkImageToImageFilter.h,v $
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Thu, 28 May 2009) $
Version:   $Revision: 17495 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKPYRAMIDALREGISTRATIONMETHOD_H
#define MITKPYRAMIDALREGISTRATIONMETHOD_H

#include "itkImageRegistrationMethod.h"
#include "MitkRigidRegistrationExports.h"
#include "itkSingleValuedNonLinearOptimizer.h"

#include "mitkImageToImageFilter.h"
#include "mitkImageAccessByItk.h"
#include "mitkRigidRegistrationObserver.h"
#include "mitkCommon.h"
#include "mitkOptimizerParameters.h"
#include "mitkTransformParameters.h"
#include "mitkMetricParameters.h"

#include "itkImageMaskSpatialObject.h"
#include "mitkRigidRegistrationTestPreset.h"



namespace mitk
{


  /*!
  \brief Main class for the rigid registration pipeline.




  \ingroup RigidRegistration

  \author Thomas van Bruggen  */
  class MITK_RIGIDREGISTRATION_EXPORT PyramidalRegistrationMethod : public ImageToImageFilter
  {

  public:

    typedef itk::SingleValuedNonLinearOptimizer         OptimizerType;
    typedef itk::ImageMaskSpatialObject< 3 >            MaskType;
       
    
    mitkClassMacro(PyramidalRegistrationMethod, ImageToImageFilter);

    itkNewMacro(Self);

    static const int LINEARINTERPOLATOR = 0;
    static const int NEARESTNEIGHBORINTERPOLATOR = 1;    

    void SetObserver(RigidRegistrationObserver::Pointer observer);

    void SetInterpolator(int interpolator);
    
    virtual void GenerateData();

    virtual void SetReferenceImage( Image::Pointer fixedImage);

    virtual void SetFixedMask( Image::Pointer fixedMask);

    virtual void SetMovingMask( Image::Pointer movingMask);

    void SetOptimizerParameters(OptimizerParameters::Pointer optimizerParameters)
    {
      m_OptimizerParameters = optimizerParameters;
    }

    OptimizerParameters::Pointer GetOptimizerParameters()
    {
      return m_OptimizerParameters;
    }

    void SetTransformParameters(TransformParameters::Pointer transformParameters)
    {
      m_TransformParameters = transformParameters;      
    }

    TransformParameters::Pointer GetTransformParameters()
    {
      return m_TransformParameters;
    }

    void SetMetricParameters(MetricParameters::Pointer metricParameters)
    {
      m_MetricParameters = metricParameters;
    }

    MetricParameters::Pointer GetMetricParameters()
    {
      return m_MetricParameters;
    }   

    void SetPresets(std::vector<std::string> presets)
    {
      m_Presets = presets;
    }
    
    itkSetMacro(FixedSchedule, itk::Array2D<unsigned int>);
    itkSetMacro(MovingSchedule, itk::Array2D<unsigned int>);
    itkSetMacro(MatchHistograms, bool);
    itkGetMacro(Preset, mitk::RigidRegistrationTestPreset*);
    itkSetMacro(BlurFixedImage, bool);
    itkSetMacro(BlurMovingImage, bool);
   

  protected:
    PyramidalRegistrationMethod();
    virtual ~PyramidalRegistrationMethod();

    template < typename TPixel, unsigned int VImageDimension >
      void GenerateData2( itk::Image<TPixel, VImageDimension>* itkImage1);

    RigidRegistrationObserver::Pointer m_Observer;
    int m_Interpolator;
    Image::Pointer m_ReferenceImage;
    Image::Pointer m_FixedMask;
    Image::Pointer m_MovingMask;

    void GenerateOutputInformation(){};
    

  private:
    OptimizerParameters::Pointer m_OptimizerParameters;
    TransformParameters::Pointer m_TransformParameters;
    MetricParameters::Pointer m_MetricParameters;

    std::vector<std::string> m_Presets;
    mitk::RigidRegistrationTestPreset* m_Preset;
    
    // Schedules
    itk::Array2D<unsigned int> m_FixedSchedule;
    itk::Array2D<unsigned int> m_MovingSchedule;

    
    bool m_UseMask;   
    bool m_MatchHistograms;
    bool m_BlurFixedImage;
    bool m_BlurMovingImage;
    MaskType::Pointer m_BrainMask;

    mitk::TransformParameters::Pointer ParseTransformParameters(itk::Array<double> transformValues);
    mitk::MetricParameters::Pointer ParseMetricParameters(itk::Array<double> metricValues);
    mitk::OptimizerParameters::Pointer ParseOptimizerParameters(itk::Array<double> optimizerValues);    
  
    
  };
}


#endif // MITKIMAGEREGISTRATIONMETHOD_H

