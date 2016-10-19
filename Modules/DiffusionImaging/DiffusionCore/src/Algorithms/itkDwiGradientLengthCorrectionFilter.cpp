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
/*=========================================================================

Program:   Tensor ToolKit - TTK
Module:    $URL: svn://scm.gforge.inria.fr/svn/ttk/trunk/Algorithms/itkElectrostaticRepulsionDiffusionGradientReductionFilter.txx $
Language:  C++
Date:      $Date: 2010-06-07 13:39:13 +0200 (Mo, 07 Jun 2010) $
Version:   $Revision: 68 $

Copyright (c) INRIA 2010. All rights reserved.
See LICENSE.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "itkDwiGradientLengthCorrectionFilter.h"

itk::DwiGradientLengthCorrectionFilter::DwiGradientLengthCorrectionFilter()
    : m_ReferenceBValue(0)
    , m_RoundingValue(0)
    , m_ReferenceGradientDirectionContainer(nullptr)
    , m_OutputGradientDirectionContainer(nullptr)
{
}

itk::DwiGradientLengthCorrectionFilter::~DwiGradientLengthCorrectionFilter()
{
}

void itk::DwiGradientLengthCorrectionFilter::GenerateData()
{

    if(m_ReferenceBValue == 0 || m_RoundingValue == 0 || m_ReferenceGradientDirectionContainer.IsNull())
        itkExceptionMacro("Wrong initialization");

    m_OutputGradientDirectionContainer = GradientDirectionContainerType::New();

    m_NewBValue = itk::NumericTraits<double>::max();
    GradientDirectionContainerType::ConstIterator it = m_ReferenceGradientDirectionContainer->Begin();
    for(; it != m_ReferenceGradientDirectionContainer->End(); ++it)
    {
        const double twonorm = it.Value().two_norm();
        const double currentBValue = m_ReferenceBValue*twonorm*twonorm ;
        const double roundedBValue = int((currentBValue + 0.5 * m_RoundingValue)/m_RoundingValue)*m_RoundingValue;

        if (roundedBValue<m_NewBValue && roundedBValue>1)
            m_NewBValue = roundedBValue;
    }

    it = m_ReferenceGradientDirectionContainer->Begin();
    for(; it != m_ReferenceGradientDirectionContainer->End(); ++it)
    {
        const double twonorm = it.Value().two_norm();
        const double currentBValue = m_ReferenceBValue*twonorm*twonorm ;
        const double roundedBValue = int((currentBValue + 0.5 * m_RoundingValue)/m_RoundingValue)*m_RoundingValue;

        if (roundedBValue>1)
        {
            const double f = std::sqrt(roundedBValue/m_NewBValue);
            GradientDirectionType grad = it.Value();
            m_OutputGradientDirectionContainer->push_back( grad.normalize() * f );
        }
        else
        {
            GradientDirectionType grad; grad.fill(0.0);
            m_OutputGradientDirectionContainer->push_back( grad );
        }
    }
}
