/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMeanResidenceTimeDescriptionParameter_h
#define mitkMeanResidenceTimeDescriptionParameter_h


#include <mitkCurveDescriptionParameterBase.h>

namespace  mitk
{

  /**
   * \brief Curve description parameter that computes the Mean Residence Time (MRT).
   *
   * The MRT is defined as the ratio of the Area Under the First Moment Curve (AUMC) to
   * the Area Under the Curve (AUC): MRT = AUMC / AUC.
   *
   * \sa AreaUnderTheCurveDescriptionParameter, AreaUnderFirstMomentDescriptionParameter, CurveDescriptionParameterBase
   */
    class MITKPHARMACOKINETICS_EXPORT MeanResidenceTimeDescriptionParameter : public mitk::CurveDescriptionParameterBase
    {
    public:
        typedef mitk::MeanResidenceTimeDescriptionParameter Self;
        typedef CurveDescriptionParameterBase Superclass;
        typedef itk::SmartPointer< Self >                            Pointer;
        typedef itk::SmartPointer< const Self >                      ConstPointer;

        itkFactorylessNewMacro(Self);
        itkCloneMacro(Self);

        /** \brief Returns the name of the computed parameter ("MeanResidenceTime").
         *  \return Vector containing the single parameter name. */
        DescriptionParameterNamesType GetDescriptionParameterName() const override;

    protected:
        static const std::string PARAMETER_NAME;

        MeanResidenceTimeDescriptionParameter();
        ~MeanResidenceTimeDescriptionParameter() override;

        DescriptionParameterResultsType ComputeCurveDescriptionParameter(const CurveType& curve, const CurveGridType& grid) const override;

    };
}
#endif
