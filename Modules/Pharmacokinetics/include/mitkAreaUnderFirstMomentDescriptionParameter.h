/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAreaUnderFirstMomentDescriptionParameter_h
#define mitkAreaUnderFirstMomentDescriptionParameter_h

#include <mitkCurveDescriptionParameterBase.h>

namespace  mitk
{

  /** \class AreaUnderFirstMomentDescriptionParameter
   * \brief Curve description parameter that computes the Area Under the First Moment Curve (AUMC).
   *
   * The AUMC is calculated by numerical integration using the trapezoidal rule over
   * the product of the curve values and their corresponding time points, i.e.
   * AUMC = integral( |C(t) * t| dt ). This is useful for pharmacokinetic analysis
   * to derive mean residence time (MRT = AUMC / AUC).
   *
   * \sa AreaUnderTheCurveDescriptionParameter, MeanResidenceTimeDescriptionParameter, CurveDescriptionParameterBase
   */
    class MITKPHARMACOKINETICS_EXPORT AreaUnderFirstMomentDescriptionParameter : public mitk::CurveDescriptionParameterBase
    {
    public:
        typedef mitk::AreaUnderFirstMomentDescriptionParameter Self;
        typedef CurveDescriptionParameterBase Superclass;
        typedef itk::SmartPointer< Self >                            Pointer;
        typedef itk::SmartPointer< const Self >                      ConstPointer;

        itkFactorylessNewMacro(Self);
        itkCloneMacro(Self);

        /** \brief Returns the name of the computed description parameter ("AreaUnderFirstMoment").
         *  \return Vector containing the single parameter name. */
        DescriptionParameterNamesType GetDescriptionParameterName() const override;

    protected:
        static const std::string PARAMETER_NAME;

        AreaUnderFirstMomentDescriptionParameter();
        ~AreaUnderFirstMomentDescriptionParameter() override;

        DescriptionParameterResultsType ComputeCurveDescriptionParameter(const CurveType& curve, const CurveGridType& grid) const override;

    };
}

#endif
