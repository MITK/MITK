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

#include <itkRescaleIntensityImageFilter.h>
#include <itkCastImageFilter.h>

#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>

#include "mitkRegEvaluationObject.h"

#include <mapExceptionObjectMacros.h>

namespace mitk
{

  RegEvaluationObject::RegEvaluationObject()
  {
  }

  RegEvaluationObject::~RegEvaluationObject()
  {
  }

  void RegEvaluationObject::SetRequestedRegionToLargestPossibleRegion()
  {
    //nothing to do
  }

  bool RegEvaluationObject::RequestedRegionIsOutsideOfTheBufferedRegion()
  {
    return false;
  }

  bool RegEvaluationObject::VerifyRequestedRegion()
  {
    return true;
  }

  void RegEvaluationObject::SetRequestedRegion(const itk::DataObject*)
  {
    //nothing to do
  }

  void RegEvaluationObject::SetTargetImage(const mitk::Image* tImg)
  {
    itkDebugMacro("setting TargetImage to " << tImg);

    AccessFixedDimensionByItk_n(tImg, doConversion, 3, (m_TargetImage));

    this->Modified();
  };

  void RegEvaluationObject::SetMovingImage(const mitk::Image* mImg)
  {
    itkDebugMacro("setting MovingImage to " << mImg);

    AccessFixedDimensionByItk_n(mImg, doConversion, 3, (m_MovingImage));

    this->Modified();
  };

  template <typename TPixelType, unsigned int VImageDimension >
  void RegEvaluationObject::doConversion(const ::itk::Image<TPixelType,VImageDimension>* input, mitk::Image::Pointer& result) const
  {
    typedef ::itk::Image<TPixelType, VImageDimension> InputImageType;
    typedef itk::CastImageFilter<InputImageType, InternalImageType> CastFilterType;
    typedef itk::RescaleIntensityImageFilter<InputImageType, InputImageType> RescaleFilterType;

    typename CastFilterType::Pointer caster = CastFilterType::New();
    typename RescaleFilterType::Pointer rescaler = RescaleFilterType::New();

    rescaler->SetInput(input);
    rescaler->SetOutputMinimum(0);
    rescaler->SetOutputMaximum(255);
    caster->SetInput(rescaler->GetOutput());
    caster->Update();
    InternalImageType::Pointer internalImage = caster->GetOutput();

    mitk::CastToMitkImage<>(internalImage,result);
  }


  void RegEvaluationObject::PrintSelf (std::ostream &os, itk::Indent indent) const
  {
    Superclass::PrintSelf(os,indent);
    if (m_Registration.IsNull())
    {
      os<< "Error. Eval object points to invalid registration (NULL).";
    }
    else
    {
      os<<std::endl<<indent<<"MatchPoint registration instance:";
      m_Registration->Print(os,indent.GetNextIndent());
    }

    if (m_TargetImage.IsNull())
    {
      os<< "Error. Eval object points to invalid target image (NULL).";
    }
    else
    {
      os<<std::endl<<indent<<"Target image instance:";
      m_TargetImage->Print(os,indent.GetNextIndent());
    }

    if (m_MovingImage.IsNull())
    {
      os<< "Error. Eval object points to invalid moving image (NULL).";
    }
    else
    {
      os<<std::endl<<indent<<"Moving image instance:";
      m_MovingImage->Print(os,indent.GetNextIndent());
    }

  }

}

