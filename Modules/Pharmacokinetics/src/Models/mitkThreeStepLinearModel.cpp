#include "mitkThreeStepLinearModel.h"

const std::string mitk::ThreeStepLinearModel::MODEL_DISPLAY_NAME = "Three Step Linear Model";

const std::string mitk::ThreeStepLinearModel::NAME_PARAMETER_S0 = "Baseline";
const std::string mitk::ThreeStepLinearModel::NAME_PARAMETER_t1 = "Time_point_1";
const std::string mitk::ThreeStepLinearModel::NAME_PARAMETER_t2 = "Time_point_2";
const std::string mitk::ThreeStepLinearModel::NAME_PARAMETER_a1 = "Slope_1";
const std::string mitk::ThreeStepLinearModel::NAME_PARAMETER_a2 = "Slope_2";

// Assuming that Model is calculated on Signal intensities I
const std::string mitk::ThreeStepLinearModel::UNIT_PARAMETER_S0 = "I";
const std::string mitk::ThreeStepLinearModel::UNIT_PARAMETER_t1 = "s";
const std::string mitk::ThreeStepLinearModel::UNIT_PARAMETER_t2 = "s";
const std::string mitk::ThreeStepLinearModel::UNIT_PARAMETER_a1 = "I/s";
const std::string mitk::ThreeStepLinearModel::UNIT_PARAMETER_a2 = "I/s";

const unsigned int mitk::ThreeStepLinearModel::POSITION_PARAMETER_S0 = 0;
const unsigned int mitk::ThreeStepLinearModel::POSITION_PARAMETER_t1 = 1;
const unsigned int mitk::ThreeStepLinearModel::POSITION_PARAMETER_t2 = 2;
const unsigned int mitk::ThreeStepLinearModel::POSITION_PARAMETER_a1 = 3;
const unsigned int mitk::ThreeStepLinearModel::POSITION_PARAMETER_a2 = 4;

const unsigned int mitk::ThreeStepLinearModel::NUMBER_OF_PARAMETERS = 5;

std::string mitk::ThreeStepLinearModel::GetModelDisplayName() const
{
  return MODEL_DISPLAY_NAME;
};

std::string mitk::ThreeStepLinearModel::GetModelType() const
{
  return "Generic";
};

mitk::ThreeStepLinearModel::FunctionStringType mitk::ThreeStepLinearModel::GetFunctionString() const
{
  return "Baseline if t<Time_point_1; Slope_1*t+Y_intercept_1 if Time_point_1<=t<=Time_point_2; Slope_2*t+Y_intercept_2 if Time_point_2<t";
};

std::string mitk::ThreeStepLinearModel::GetXName() const
{
  return "x";
};

mitk::ThreeStepLinearModel::ParameterNamesType
mitk::ThreeStepLinearModel::GetParameterNames() const
{
  ParameterNamesType result;
  result.push_back(NAME_PARAMETER_S0);
  result.push_back(NAME_PARAMETER_t1);
  result.push_back(NAME_PARAMETER_t2);
  result.push_back(NAME_PARAMETER_a1);
  result.push_back(NAME_PARAMETER_a2);


  return result;
};

mitk::ThreeStepLinearModel::ParametersSizeType
mitk::ThreeStepLinearModel::GetNumberOfParameters() const
{
  return NUMBER_OF_PARAMETERS;
};

mitk::ThreeStepLinearModel::ParamterUnitMapType
mitk::ThreeStepLinearModel::GetParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_PARAMETER_S0, UNIT_PARAMETER_S0));
  result.insert(std::make_pair(NAME_PARAMETER_t1, UNIT_PARAMETER_t1));
  result.insert(std::make_pair(NAME_PARAMETER_t2, UNIT_PARAMETER_t2));
  result.insert(std::make_pair(NAME_PARAMETER_a1, UNIT_PARAMETER_a1));
  result.insert(std::make_pair(NAME_PARAMETER_a2, UNIT_PARAMETER_a2));

  return result;
};

mitk::ThreeStepLinearModel::ParameterNamesType
mitk::ThreeStepLinearModel::GetDerivedParameterNames() const
{
  ParameterNamesType result;
  result.push_back("AUC");
  result.push_back("FinalTimePoint");
  result.push_back("FinalUptake");
  result.push_back("Smax");
  result.push_back("y-intercept2");
  result.push_back("y-intercept1");

  return result;
};

mitk::ThreeStepLinearModel::ParametersSizeType
mitk::ThreeStepLinearModel::GetNumberOfDerivedParameters() const
{
  return 6;
};

mitk::ThreeStepLinearModel::ParamterUnitMapType mitk::ThreeStepLinearModel::GetDerivedParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair("AUC", "I*s"));
  result.insert(std::make_pair("FinalTimePoint", "s"));
  result.insert(std::make_pair("FinalUptake", "I"));
  result.insert(std::make_pair("Smax", "I"));
  result.insert(std::make_pair("y-intercept1", "I"));
  result.insert(std::make_pair("y-intercept2", "I"));




  return result;
};

mitk::ThreeStepLinearModel::ModelResultType
mitk::ThreeStepLinearModel::ComputeModelfunction(const ParametersType& parameters) const
{
  //Model Parameters
  double     S0 = (double) parameters[POSITION_PARAMETER_S0];
  double     t1 = (double) parameters[POSITION_PARAMETER_t1] ;
  double     t2 = (double) parameters[POSITION_PARAMETER_t2] ;
  double     a1 = (double) parameters[POSITION_PARAMETER_a1] ;
  double     a2 = (double) parameters[POSITION_PARAMETER_a2] ;


  double     b1 = S0-a1*t1 ;
  double     b2 = (a1*t2+ b1) - (a2*t2);

  ModelResultType signal(m_TimeGrid.GetSize());

  TimeGridType::const_iterator timeGridEnd = m_TimeGrid.end();
  ModelResultType::iterator signalPos = signal.begin();

  for (TimeGridType::const_iterator gridPos = m_TimeGrid.begin(); gridPos != timeGridEnd; ++gridPos, ++signalPos)
  {
      if((*gridPos) < t1)
      {
          *signalPos = S0;
      }
      else if ((*gridPos) >= t1 && (*gridPos) <= t2)
      {
          *signalPos = a1*(*gridPos)+b1;
      }
      else
      {
          *signalPos = a2*(*gridPos)+b2;
      }
  }

  return signal;
};

mitk::ThreeStepLinearModel::ParameterNamesType mitk::ThreeStepLinearModel::GetStaticParameterNames() const
{
  ParameterNamesType result;

  return result;
}

mitk::ThreeStepLinearModel::ParametersSizeType  mitk::ThreeStepLinearModel::GetNumberOfStaticParameters() const
{
  return 0;
}

void mitk::ThreeStepLinearModel::SetStaticParameter(const ParameterNameType&,
    const StaticParameterValuesType&)
{
  //do nothing
};

mitk::ThreeStepLinearModel::StaticParameterValuesType mitk::ThreeStepLinearModel::GetStaticParameterValue(
  const ParameterNameType&) const
{
  StaticParameterValuesType result;

  //do nothing

  return result;
};

mitk::ModelBase::DerivedParameterMapType mitk::ThreeStepLinearModel::ComputeDerivedParameters(
  const mitk::ModelBase::ParametersType& parameters) const
{
    double     S0 = (double) parameters[POSITION_PARAMETER_S0];
    double     tau = (double) parameters[POSITION_PARAMETER_t1] ;
    double     tmax = (double) parameters[POSITION_PARAMETER_t2] ;
    double     s1 = (double) parameters[POSITION_PARAMETER_a1] ;
    double     s2 = (double) parameters[POSITION_PARAMETER_a2] ;

    unsigned int timeSteps = m_TimeGrid.GetSize();

    double Taq = 0;
    Taq = m_TimeGrid.GetElement(timeSteps-1);

    double b1 = S0-s1*tau;
    double Smax = s1*tmax+b1;
    double b2 = Smax-s2*tmax;
    double AUC = s1/2*(tmax*tmax-tau*tau)
                 +(b1-S0)*(tmax-tau)
                 +s2/2*(Taq*Taq-tmax*tmax)
                 +(b2-S0)*(Taq-tmax);
    double Sfin = s2*Taq+b2;

    DerivedParameterMapType result;

    result.insert(std::make_pair("AUC", AUC));
    result.insert(std::make_pair("FinalTimePoint", Taq));
    result.insert(std::make_pair("FinalUptake", Sfin));
    result.insert(std::make_pair("Smax", Smax));
    result.insert(std::make_pair("y-intercept1", b1));
    result.insert(std::make_pair("y-intercept2", b2));

    return result;
};

itk::LightObject::Pointer mitk::ThreeStepLinearModel::InternalClone() const
{
  ThreeStepLinearModel::Pointer newClone = ThreeStepLinearModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);

  return newClone.GetPointer();
};
