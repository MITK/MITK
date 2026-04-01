/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTimeToPeakCurveDescriptionParameter_h
#define mitkTimeToPeakCurveDescriptionParameter_h
#include <mitkCurveDescriptionParameterBase.h>

namespace mitk {

/**
 * \brief Curve description parameter that computes the time to peak and the peak value.
 *
 * Returns two values: the time point at which the first maximum of the curve occurs
 * and the maximum curve value itself.
 *
 * \sa MaximumCurveDescriptionParameter, CurveDescriptionParameterBase
 */
class MITKPHARMACOKINETICS_EXPORT TimeToPeakCurveDescriptionParameter : public mitk::CurveDescriptionParameterBase
{
public:
    typedef mitk::TimeToPeakCurveDescriptionParameter Self;
    typedef CurveDescriptionParameterBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Returns the names of the computed parameters ("TimeToPeak", "PeakValue").
     *  \return Vector of parameter name strings. */
    DescriptionParameterNamesType GetDescriptionParameterName() const override;


protected:
    static const std::string PARAMETER_PEAK_NAME;
    static const std::string PARAMETER_TIME_NAME;

    TimeToPeakCurveDescriptionParameter();
    ~TimeToPeakCurveDescriptionParameter() override;

    DescriptionParameterResultsType ComputeCurveDescriptionParameter(const CurveType& curve, const CurveGridType& grid) const override;
};



}

#endif
