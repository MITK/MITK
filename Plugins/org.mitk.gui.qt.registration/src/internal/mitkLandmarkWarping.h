/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKLANDMARKWARPING_H
#define MITKLANDMARKWARPING_H

#include "itkDeformationFieldSource.h"
#include "itkWarpImageFilter.h"
#include "itkCommand.h"
#include "mitkProgressBar.h"

namespace mitk
{
  class LandmarkWarping
  {

  public:
    LandmarkWarping();
    ~LandmarkWarping();

    static const unsigned int Dimension = 3;
    typedef   float VectorComponentType;

    typedef   itk::Vector< VectorComponentType, Dimension >    VectorType;

    typedef   itk::Image< VectorType,  Dimension >   DeformationFieldType;


    typedef   unsigned char  PixelType;
    typedef   itk::Image< PixelType, Dimension >                                                   FixedImageType;
    typedef   itk::Image< PixelType, Dimension >                                                   MovingImageType;
    typedef   itk::DeformationFieldSource< DeformationFieldType >                                  DeformationSourceType;
    typedef   DeformationSourceType::LandmarkContainerPointer                                      LandmarkContainerPointer;
    typedef   DeformationSourceType::LandmarkContainer                                             LandmarkContainerType;
    typedef   DeformationSourceType::LandmarkPointType                                             LandmarkPointType;
    typedef   itk::WarpImageFilter< MovingImageType, MovingImageType, DeformationFieldType  >      FilterType;

    FixedImageType::Pointer m_FixedImage;
    MovingImageType::Pointer m_MovingImage;
    DeformationSourceType::Pointer m_Deformer;
    DeformationSourceType::Pointer m_LandmarkDeformer;
    LandmarkWarping::LandmarkContainerType::Pointer m_TargetLandmarks;
    LandmarkWarping::LandmarkContainerType::Pointer m_SourceLandmarks;
    FilterType::Pointer m_Warper;
    DeformationFieldType::ConstPointer m_DeformationField;
    DeformationFieldType::ConstPointer m_InverseDeformationField;


    void SetFixedImage(FixedImageType::Pointer fi);
    void SetMovingImage(MovingImageType::Pointer mi);
    void SetLandmarks(LandmarkContainerType::Pointer source, LandmarkContainerType::Pointer target);
    MovingImageType::Pointer Register();
    LandmarkContainerType::Pointer GetTransformedTargetLandmarks();

  protected:

    // observerclass to react on changes on pointsetnodes
    class Observer : public itk::Command 
    {
    public:
      typedef  Observer                 Self;
      typedef  itk::Command             Superclass;
      typedef  itk::SmartPointer<Self>  Pointer;

      itkNewMacro( Self );

    protected:
      Observer() 
      {
        m_Counter = 0;
      };
      unsigned int m_Counter;

    public:

      void Execute(itk::Object *object, const itk::EventObject & event)
      {
        Execute( (const itk::Object*) object, event );
      }

      void Execute(const itk::Object * object, const itk::EventObject & /*event*/)
      {
        const itk::ProcessObject * internalFilter = dynamic_cast<const itk::ProcessObject *>( object );
        if (!(internalFilter->GetProgress() == 0 || internalFilter->GetProgress() == 1)) 
        {
          ++m_Counter;
          mitk::ProgressBar::GetInstance()->Progress();
        }
        if (internalFilter->GetProgress() == 1 && m_Counter > 20 && m_Counter < 120)
        {
          mitk::ProgressBar::GetInstance()->Progress(120 - m_Counter);
          m_Counter = 120;
        }
      }
    };

    Observer::Pointer m_Observer;
  };
}

#endif // MITKLANDMARKWARPING_H
