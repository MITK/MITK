/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMaximumCurveDescriptionParameter_h
#define mitkMaximumCurveDescriptionParameter_h

#include <mitkCurveDescriptionParameterBase.h>

namespace mitk {

  /**
   * \brief Curve description parameter that computes the maximum value of a curve.
   *
   * \sa CurveDescriptionParameterBase
   */
class MITKPHARMACOKINETICS_EXPORT MaximumCurveDescriptionParameter : public mitk::CurveDescriptionParameterBase
{
public:
    typedef mitk::MaximumCurveDescriptionParameter Self;
    typedef CurveDescriptionParameterBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Returns the name of the computed parameter ("Maximum").
     *  \return Vector containing the single parameter name. */
    DescriptionParameterNamesType GetDescriptionParameterName() const override;

protected:
    static const std::string PARAMETER_NAME;

    MaximumCurveDescriptionParameter();
    ~MaximumCurveDescriptionParameter() override;

    DescriptionParameterResultsType ComputeCurveDescriptionParameter(const CurveType& curve, const CurveGridType& grid) const override;
};



}
#endif
