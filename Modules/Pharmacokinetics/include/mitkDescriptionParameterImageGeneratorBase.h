/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDescriptionParameterImageGeneratorBase_h
#define mitkDescriptionParameterImageGeneratorBase_h

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
    ~DescriptionParameterImageGeneratorBase() override;

    virtual bool HasOutdatedResult() const;

    virtual void CheckValidInputs() const;
    virtual void DoParameterCalculationAndGetResults(ParameterImageMapType& parameterImages) = 0;

    itk::TimeStamp m_GenerationTimeStamp;

  private:

    ParameterImageMapType m_ParameterImageMap;

  };

}

#endif
