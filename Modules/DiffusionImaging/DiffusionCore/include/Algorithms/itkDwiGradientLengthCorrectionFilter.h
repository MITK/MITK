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

#ifndef _itk_DwiGradientLengthCorrectionFilter_h_
#define _itk_DwiGradientLengthCorrectionFilter_h_

#include <itkProcessObject.h>
#include <vnl/vnl_vector_fixed.h>
#include <itkVectorContainer.h>
#include <MitkDiffusionCoreExports.h>

namespace itk
{

class MITKDIFFUSIONCORE_EXPORT DwiGradientLengthCorrectionFilter : public ProcessObject
{

public:
    typedef DwiGradientLengthCorrectionFilter                                                           Self;
    typedef SmartPointer<Self>                                                                          Pointer;
    typedef SmartPointer<const Self>                                                                    ConstPointer;
    typedef ProcessObject                                                                               Superclass;

    typedef vnl_vector_fixed< double, 3 >                                                               GradientDirectionType;
    typedef itk::VectorContainer< unsigned int, GradientDirectionType >                                 GradientDirectionContainerType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    itkTypeMacro(DwiGradientLengthCorrectionFilter,ProcessObject)

    void GenerateData() override;

    void Update() override{
        this->GenerateData();
    }

    // input
    itkSetMacro(RoundingValue, int)
    itkSetMacro(ReferenceBValue, double)
    itkSetMacro(ReferenceGradientDirectionContainer, GradientDirectionContainerType::Pointer)

    // output
    itkGetMacro(OutputGradientDirectionContainer, GradientDirectionContainerType::Pointer)
    itkGetMacro(NewBValue, double)

    protected:
        DwiGradientLengthCorrectionFilter();
    ~DwiGradientLengthCorrectionFilter() override;

    double m_NewBValue;
    double m_ReferenceBValue;
    int m_RoundingValue;

    GradientDirectionContainerType::Pointer m_ReferenceGradientDirectionContainer;
    GradientDirectionContainerType::Pointer m_OutputGradientDirectionContainer;
};


} // end of namespace

#endif
