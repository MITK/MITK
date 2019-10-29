#include "mitkMeanResidenceTimeDescriptionParameter.h"

const std::string mitk::MeanResidenceTimeDescriptionParameter::PARAMETER_NAME = "MeanResidenceTime";

mitk::MeanResidenceTimeDescriptionParameter::MeanResidenceTimeDescriptionParameter()
{
}

mitk::MeanResidenceTimeDescriptionParameter::~MeanResidenceTimeDescriptionParameter()
{
}

mitk::CurveDescriptionParameterBase::DescriptionParameterNamesType mitk::MeanResidenceTimeDescriptionParameter::GetDescriptionParameterName() const
{
  DescriptionParameterNamesType result = { PARAMETER_NAME };
  return result;
}

mitk::CurveDescriptionParameterBase::DescriptionParameterResultsType mitk::MeanResidenceTimeDescriptionParameter::ComputeCurveDescriptionParameter(const CurveType &curve, const CurveGridType &grid) const
{
  DescriptionParameterResultsType results;
  CurveDescriptionParameterResultType aumc = 0;
  CurveDescriptionParameterResultType auc = 0;
  CurveDescriptionParameterResultType mrt = 0;

  auto steps = grid.GetSize();

  for (CurveGridType::size_type i = 0; i < steps - 1; ++i)
  {
    double deltaX = grid(i + 1) - grid(i);

    double deltaY1 = curve(i + 1)*grid(i + 1) - curve(i)*grid(i);
    double Y1 = curve(i)*grid(i);
    double intI1 = 1 / 2 * deltaX * deltaY1 + Y1 * deltaX;
    aumc += std::abs(intI1);


    double deltaY2 = curve(i + 1) - curve(i);
    double Y2 = curve(i);
    double intI2 = 1 / 2 * deltaX * deltaY2 + Y2 * deltaX;
    auc += std::abs(intI2);


  }
  if(auc != 0)
  {
    mrt = aumc/auc;
  }
  results.push_back(mrt);
  return results;
}
