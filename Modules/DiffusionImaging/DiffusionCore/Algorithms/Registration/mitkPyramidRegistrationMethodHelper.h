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

#ifndef MITKPYRAMIDREGISTRATIONMETHODHELPER_H
#define MITKPYRAMIDREGISTRATIONMETHODHELPER_H

#include <MitkDiffusionCoreExports.h>

#include <itkCommand.h>

#include <itkRegularStepGradientDescentOptimizer.h>
#include <itkMattesMutualInformationImageToImageMetricv4.h>
#include <itkCorrelationImageToImageMetricv4.h>
#include <itkGradientDescentLineSearchOptimizerv4.h>

#include <itkImageRegistrationMethodv4.h>

#include <itkAffineTransform.h>
#include <itkEuler3DTransform.h>

#include <itkMattesMutualInformationImageToImageMetric.h>
#include <itkNormalizedCorrelationImageToImageMetric.h>

#include <itkMultiResolutionImageRegistrationMethod.h>
#include <itkImageMomentsCalculator.h>

#include "mitkImageAccessByItk.h"

/**
 * @brief Provides same functionality as \a AccessTwoImagesFixedDimensionByItk for a subset of types
 *
 *  For now, the subset defined is only short and float.
 */
#define AccessTwoImagesFixedDimensionTypeSubsetByItk(mitkImage1, mitkImage2, itkImageTypeFunction, dimension) \
{                                                                                                   \
  const mitk::PixelType& pixelType1 = mitkImage1->GetPixelType();                                   \
  const mitk::PixelType& pixelType2 = mitkImage2->GetPixelType();                                   \
  const mitk::Image* constImage1 = mitkImage1;                                                      \
  const mitk::Image* constImage2 = mitkImage2;                                                      \
  mitk::Image* nonConstImage1 = const_cast<mitk::Image*>(constImage1);                              \
  mitk::Image* nonConstImage2 = const_cast<mitk::Image*>(constImage2);                              \
  nonConstImage1->Update();                                                                         \
  nonConstImage2->Update();                                                                         \
  _checkSpecificDimension(mitkImage1, (dimension));                                                 \
  _checkSpecificDimension(mitkImage2, (dimension));                                                 \
  _accessTwoImagesByItkForEach(itkImageTypeFunction, ((short, dimension))((unsigned short, dimension))((float, dimension))((double, dimension)), ((short, dimension))((unsigned short, dimension))((float, dimension))((double, dimension)) ) \
  {                                                                                                 \
    std::string msg("Pixel type ");                                                                 \
    msg.append(pixelType1.GetComponentTypeAsString() );                                             \
    msg.append(" or pixel type ");                                                                  \
    msg.append(pixelType2.GetComponentTypeAsString() );                                             \
    msg.append(" is not in " MITK_PP_STRINGIZE(MITK_ACCESSBYITK_TYPES_DIMN_SEQ(dimension)));        \
    throw mitk::AccessByItkException(msg);                                                          \
  }                                                                                                 \
}


/**
 * @brief The PyramidOptControlCommand class stears the step lenght of the
 * gradient descent optimizer in multi-scale registration methods
 */
template <typename RegistrationType >
class PyramidOptControlCommand : public itk::Command
{
public:

  typedef itk::RegularStepGradientDescentOptimizer OptimizerType;

  mitkClassMacro(PyramidOptControlCommand<RegistrationType>, itk::Command)
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  void Execute(itk::Object *caller, const itk::EventObject & /*event*/)
  {
    RegistrationType* registration = dynamic_cast< RegistrationType* >( caller );

    if( registration == NULL)
      return;

    MITK_DEBUG << "\t - Pyramid level " << registration->GetCurrentLevel();
    if( registration->GetCurrentLevel() == 0 )
    { MITK_WARN("OptCommand") << "Cast to registration failed";
          return;
    }

    OptimizerType* optimizer = dynamic_cast< OptimizerType* >(registration->GetOptimizer());

    if( optimizer == NULL)
    { MITK_WARN("OptCommand") << "Cast to optimizer failed";
          return;
    }

    MITK_INFO /*<< optimizer->GetStopConditionDescription()  << "\n"*/
               << optimizer->GetValue() << " : " << optimizer->GetCurrentPosition();

    optimizer->SetMaximumStepLength( optimizer->GetMaximumStepLength() * 0.25f );
    optimizer->SetMinimumStepLength( optimizer->GetMinimumStepLength() * 0.1f );
   // optimizer->SetNumberOfIterations( optimizer->GetNumberOfIterations() * 1.5f );
  }

  void Execute(const itk::Object * /*object*/, const itk::EventObject & /*event*/){}
};

#include <itkGradientDescentLineSearchOptimizerv4.h>

template <typename RegistrationType >
class PyramidOptControlCommandv4 : public itk::Command
{
public:

  typedef itk::GradientDescentLineSearchOptimizerv4 OptimizerType;

  mitkClassMacro(PyramidOptControlCommandv4<RegistrationType>, itk::Command)
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  void Execute(itk::Object *caller, const itk::EventObject & /*event*/)
  {
    RegistrationType* registration = dynamic_cast< RegistrationType* >( caller );

    if( registration == NULL)
      return;

    MITK_DEBUG << "\t - Pyramid level " << registration->GetCurrentLevel();
    if( registration->GetCurrentLevel() == 0 )
      return;

    OptimizerType* optimizer = dynamic_cast< OptimizerType* >( registration->GetOptimizer() );

    if( optimizer == NULL)
    { MITK_WARN("OptCommand4") << "Cast to optimizer failed";
          return;
    }

    optimizer->SetNumberOfIterations( optimizer->GetNumberOfIterations() * 2.5 );
    optimizer->SetMaximumStepSizeInPhysicalUnits( optimizer->GetMaximumStepSizeInPhysicalUnits() * 0.4);

    MITK_INFO("Pyramid.Command.Iter") << optimizer->GetNumberOfIterations();
    MITK_INFO("Pyramid.Command.MaxStep") << optimizer->GetMaximumStepSizeInPhysicalUnits();

  }

  void Execute(const itk::Object * /*object*/, const itk::EventObject & /*event*/){}
};


template <typename OptimizerType>
class OptimizerIterationCommand : public itk::Command
{
public:
  mitkClassMacro(OptimizerIterationCommand<OptimizerType>, itk::Command)
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  void Execute(itk::Object *caller, const itk::EventObject & /*event*/)
  {

    OptimizerType* optimizer = dynamic_cast< OptimizerType* >( caller );

    unsigned int currentIter = optimizer->GetCurrentIteration();
    MITK_INFO << "[" << currentIter << "] : " << optimizer->GetValue() << " : " << optimizer->GetCurrentPosition();

  }

  void Execute(const itk::Object * /*object*/, const itk::EventObject & /*event*/)
  {

  }
};

template <typename OptimizerType>
class OptimizerIterationCommandv4 : public itk::Command
{
public:
  itkNewMacro( OptimizerIterationCommandv4 )

  void Execute(itk::Object *object, const itk::EventObject & event)
  {
    OptimizerType* optimizer = dynamic_cast< OptimizerType* >( object );

    if( typeid( event ) != typeid( itk::IterationEvent ) )
      { return; }

    unsigned int currentIter = optimizer->GetCurrentIteration();
    MITK_INFO << "[" << currentIter << "] : " << optimizer->GetCurrentMetricValue() << " : "
              << optimizer->GetMetric()->GetParameters() ;
              //<< " : " << optimizer->GetScales();

  }

  void Execute(const itk::Object * object, const itk::EventObject & event)
  {


  }
};


#endif // MITKPYRAMIDREGISTRATIONMETHODHELPER_H
