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

#include "mitkTwoCompartmentExchangeModel.h"
#include "mitkConvolutionHelper.h"
#include <fstream>

const std::string mitk::TwoCompartmentExchangeModel::MODEL_DISPLAY_NAME =
 "Two Compartment Exchange Model";

const std::string mitk::TwoCompartmentExchangeModel::NAME_PARAMETER_F = "F";
const std::string mitk::TwoCompartmentExchangeModel::NAME_PARAMETER_PS = "PS";
const std::string mitk::TwoCompartmentExchangeModel::NAME_PARAMETER_ve = "ve";
const std::string mitk::TwoCompartmentExchangeModel::NAME_PARAMETER_vp = "vp";

const std::string mitk::TwoCompartmentExchangeModel::UNIT_PARAMETER_F = "ml/min/100ml";
const std::string mitk::TwoCompartmentExchangeModel::UNIT_PARAMETER_PS = "ml/min/100ml";
const std::string mitk::TwoCompartmentExchangeModel::UNIT_PARAMETER_ve = "ml/ml";
const std::string mitk::TwoCompartmentExchangeModel::UNIT_PARAMETER_vp = "ml/ml";

const unsigned int mitk::TwoCompartmentExchangeModel::POSITION_PARAMETER_F = 0;
const unsigned int mitk::TwoCompartmentExchangeModel::POSITION_PARAMETER_PS = 1;
const unsigned int mitk::TwoCompartmentExchangeModel::POSITION_PARAMETER_ve = 2;
const unsigned int mitk::TwoCompartmentExchangeModel::POSITION_PARAMETER_vp = 3;

const unsigned int mitk::TwoCompartmentExchangeModel::NUMBER_OF_PARAMETERS = 4;


inline double square(double a)
{
  return a * a;
}

std::string mitk::TwoCompartmentExchangeModel::GetModelDisplayName() const
{
  return MODEL_DISPLAY_NAME;
};

std::string mitk::TwoCompartmentExchangeModel::GetModelType() const
{
  return "Perfusion.MR";
};

mitk::TwoCompartmentExchangeModel::TwoCompartmentExchangeModel()
{

}

mitk::TwoCompartmentExchangeModel::~TwoCompartmentExchangeModel()
{

}

mitk::TwoCompartmentExchangeModel::ParameterNamesType
mitk::TwoCompartmentExchangeModel::GetParameterNames() const
{
  ParameterNamesType result;

  result.push_back(NAME_PARAMETER_F);
  result.push_back(NAME_PARAMETER_PS);
  result.push_back(NAME_PARAMETER_ve);
  result.push_back(NAME_PARAMETER_vp);

  return result;
}

mitk::TwoCompartmentExchangeModel::ParametersSizeType
mitk::TwoCompartmentExchangeModel::GetNumberOfParameters() const
{
  return NUMBER_OF_PARAMETERS;
}

mitk::TwoCompartmentExchangeModel::ParamterUnitMapType
mitk::TwoCompartmentExchangeModel::GetParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_PARAMETER_F, UNIT_PARAMETER_F));
  result.insert(std::make_pair(NAME_PARAMETER_PS, UNIT_PARAMETER_PS));
  result.insert(std::make_pair(NAME_PARAMETER_vp, UNIT_PARAMETER_vp));
  result.insert(std::make_pair(NAME_PARAMETER_ve, UNIT_PARAMETER_ve));

  return result;
};


mitk::TwoCompartmentExchangeModel::ModelResultType
mitk::TwoCompartmentExchangeModel::ComputeModelfunction(const ParametersType& parameters) const
{
    typedef mitk::ModelBase::ModelResultType ConvolutionResultType;

    if (this->m_TimeGrid.GetSize() == 0)
    {
    itkExceptionMacro("No Time Grid Set! Cannot Calculate Signal");
    }

    AterialInputFunctionType aterialInputFunction;
    aterialInputFunction = GetAterialInputFunction(this->m_TimeGrid);

    unsigned int timeSteps = this->m_TimeGrid.GetSize();
    mitk::ModelBase::ModelResultType signal(timeSteps);
    signal.fill(0.0);

    //Model Parameters
    double F = parameters[POSITION_PARAMETER_F] / 6000.0;
    double PS  = parameters[POSITION_PARAMETER_PS] / 6000.0;
    double ve = parameters[POSITION_PARAMETER_ve];
    double vp = parameters[POSITION_PARAMETER_vp];

    if(PS != 0)
    {
        double Tp = vp/(PS + F);
        double Te = ve/PS;
        double Tb = vp/F;

        double Kp = 0.5 *( 1/Tp + 1/Te + sqrt(( 1/Tp + 1/Te )*( 1/Tp + 1/Te ) - 4 * 1/Te*1/Tb) );
        double Km = 0.5 *( 1/Tp + 1/Te - sqrt(( 1/Tp + 1/Te )*( 1/Tp + 1/Te ) - 4 * 1/Te*1/Tb) );

        double E = ( Kp - 1/Tb )/( Kp - Km );



        ConvolutionResultType expp = mitk::convoluteAIFWithExponential(this->m_TimeGrid, aterialInputFunction,Kp);
        ConvolutionResultType expm = mitk::convoluteAIFWithExponential(this->m_TimeGrid, aterialInputFunction,Km);

        //Signal that will be returned by ComputeModelFunction

        mitk::ModelBase::ModelResultType::const_iterator exppPos = expp.begin();
        mitk::ModelBase::ModelResultType::const_iterator expmPos = expm.begin();

        for( mitk::ModelBase::ModelResultType::iterator signalPos = signal.begin(); signalPos!=signal.end(); ++exppPos,++expmPos, ++signalPos)
        {
            *signalPos = F * ( *exppPos + E*(*expmPos - *exppPos) );
        }
    }


    else
    {
        double Kp = F/vp;
        ConvolutionResultType exp = mitk::convoluteAIFWithExponential(this->m_TimeGrid, aterialInputFunction,Kp);
        mitk::ModelBase::ModelResultType::const_iterator expPos = exp.begin();

        for( mitk::ModelBase::ModelResultType::iterator signalPos = signal.begin(); signalPos!=signal.end(); ++expPos, ++signalPos)
        {
            *signalPos = F * ( *expPos );
        }

    }

    return signal;
}


itk::LightObject::Pointer mitk::TwoCompartmentExchangeModel::InternalClone() const
{
  TwoCompartmentExchangeModel::Pointer newClone = TwoCompartmentExchangeModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);

  return newClone.GetPointer();
}

void mitk::TwoCompartmentExchangeModel::PrintSelf(std::ostream& os, ::itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);


}


