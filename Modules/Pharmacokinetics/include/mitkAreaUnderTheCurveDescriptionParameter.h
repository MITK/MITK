/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAreaUnderTheCurveDescriptionParameter_h
#define mitkAreaUnderTheCurveDescriptionParameter_h

#include <mitkCurveDescriptionParameterBase.h>

namespace  mitk
{

  /** \class AreaUnderTheCurveDescriptionParameter
   * \brief Curve description parameter that computes the Area Under the Curve (AUC).
   *
   * The AUC is computed by numerical integration using the trapezoidal rule over
   * the absolute values of the curve, i.e. AUC = integral( |C(t)| dt ).
   *
   * \sa AreaUnderFirstMomentDescriptionParameter, MeanResidenceTimeDescriptionParameter, CurveDescriptionParameterBase
   */
    class MITKPHARMACOKINETICS_EXPORT AreaUnderTheCurveDescriptionParameter : public mitk::CurveDescriptionParameterBase
    {
    public:
        typedef mitk::AreaUnderTheCurveDescriptionParameter Self;
        typedef CurveDescriptionParameterBase Superclass;
        typedef itk::SmartPointer< Self >                            Pointer;
        typedef itk::SmartPointer< const Self >                      ConstPointer;

        itkFactorylessNewMacro(Self);
        itkCloneMacro(Self);

        /** \brief Returns the name of the computed description parameter ("AreaUnderCurve").
         *  \return Vector containing the single parameter name. */
        DescriptionParameterNamesType GetDescriptionParameterName() const override;

    protected:
        static const std::string PARAMETER_NAME;

        AreaUnderTheCurveDescriptionParameter();
        ~AreaUnderTheCurveDescriptionParameter() override;

        DescriptionParameterResultsType ComputeCurveDescriptionParameter(const CurveType& curve, const CurveGridType& grid) const override;

    };
}

#endif
