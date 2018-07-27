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

#ifndef __MITK_DESCRIPTION_PARAMETER_IMAGE_GENERATOR_BASE_H_
#define __MITK_DESCRIPTION_PARAMETER_IMAGE_GENERATOR_BASE_H_

#include <map>

#include "mitkImage.h"

#include "mitkCurveDescriptionParameterBase.h"

#include "MitkPharmacokineticsExports.h"

namespace mitk
{
  class MITKPHARMACOKINETICS_EXPORT DescriptionParameterImageGeneratorBase: public ::itk::Object
  {
  public:

    mitkClassMacroItkParent(DescriptionParameterImageGeneratorBase, ::itk::Object);

    typedef ScalarType ParameterImagePixelType;

    typedef CurveDescriptionParameterBase::CurveDescriptionParameterNameType ParameterNameType;

    typedef std::map<ParameterNameType,Image::Pointer> ParameterImageMapType;

    virtual double GetProgress() const = 0;

     void Generate();

    ParameterImageMapType GetParameterImages();

  protected:
    DescriptionParameterImageGeneratorBase();
    virtual ~DescriptionParameterImageGeneratorBase();

    virtual bool HasOutdatedResult() const;

    virtual void CheckValidInputs() const;
    virtual void DoParameterCalculationAndGetResults(ParameterImageMapType& parameterImages) = 0;

    itk::TimeStamp m_GenerationTimeStamp;

  private:

    ParameterImageMapType m_ParameterImageMap;

  };

}

#endif //__MITK_DESCRIPTION_PARAMETER_IMAGE_GENERATOR_BASE_H_
