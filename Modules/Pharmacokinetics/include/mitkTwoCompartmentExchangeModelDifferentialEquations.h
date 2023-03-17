/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTwoCompartmentExchangeModelDifferentialEquations_h
#define mitkTwoCompartmentExchangeModelDifferentialEquations_h

#include "mitkNumericTwoCompartmentExchangeModel.h"

namespace mitk{
/** @class TwoCompartmentExchangeModelDifferentialEquations
 * @brief Helper Class for NumericTwoCompartmentExchangeModel: Defines the differential equations (Mass Balance Equations) in the 2 Compartment Exchange model.
 * The 2 Compartment Exchange model is defined via the mass balance equations
 * vp * dCp(t)/dt =  F * (CA(t) - Cp(t)) - PS * (Cp(t) - Ce(t))
 * ve * dCe(t)/dt = PS * (Cp(t) - Ce(t))
 * Boost ODEINT performs a stepwise numeric integration (e.g. via Runge-Kutta method) of the initial value problem
 * x' = dx/dt = f(x,t)
 * It needs an operator () (a functor) that calculates dx/dt = dxdt for a given x and t.
 * Parameters are F, PS, ve and vp and the time dependent Ca(t) =AIF, that is interpolated to the current step t
*/

class TwoCompartmentExchangeModelDifferentialEquations
{
public:

    typedef std::vector< double > AIFType;

    /** @brief Functor for differential equation of Physiological Pharmacokinetic Brix Model
     * Takes current state x = x(t) and time t and calculates the corresponding dxdt = dx/dt
    */
    void operator() (const mitk::NumericTwoCompartmentExchangeModel::state_type &x, mitk::NumericTwoCompartmentExchangeModel::state_type &dxdt, const double t)
    {
        double Ca_t = InterpolateAIFToCurrentTimeStep(t);

//        dxdt[0] = -(this->FVP + this->PSVP)*x[0] - this->PSVP*x[1]+this->FVP*Ca_t;
        dxdt[0] = (1/this->vp) * ( this->F*(Ca_t - x[0]) - this->PS*(x[0] - x[1]) );
        dxdt[1] = (1/this->ve) * this->PS * (x[0] - x[1]);

    }

    TwoCompartmentExchangeModelDifferentialEquations() : F(0), PS(0), ve(0), vp(0), m_AIF(0), m_AIFTimeGrid(0)
    {
    }

    /** @brief Initialize class with parameters F/Vp, PS/Vp, fi and fp that are free fit parameters*/
    void initialize(double Fp, double ps, double fi, double fp)
    {
        this->F = Fp;
        this->PS = ps;
        this->ve = fi;
        this->vp = fp;
    }


    void setAIF(AIFType &aif)
    {
        this->m_AIF = aif;
    }


    void setAIFTimeGrid(AIFType &grid)
    {
        this->m_AIFTimeGrid = grid;
    }

private:

    double F;
    double PS;
    double ve;
    double vp;

    AIFType m_AIF;
    AIFType m_AIFTimeGrid;


    /** @brief Internal routine to interpolate the AIF to the current time point t used for integration
     * The numerical integration of ODEINT is performed on an adaptive timegrid (adaptive step size dt) different from the time grid of the AIF and model function.
     * Thus, the AIF value Ca(t) has to be interpolated from the set AIF
     */
    double InterpolateAIFToCurrentTimeStep(double t)
    {
        double lastValue = m_AIF[0];
        double lastTime = std::numeric_limits<double>::min();

        AIFType::const_iterator posITime = m_AIFTimeGrid.begin();
        AIFType::const_iterator posValue = m_AIF.begin();

        while(t > *posITime)
        {
            lastValue = *posValue;
            lastTime = *posITime;
            ++posValue;
            ++posITime;
        }
        double weightLast = 1 - (t - lastTime)/(*posITime - lastTime);
        double weightNext = 1- (*posITime - t)/(*posITime - lastTime);
        double result = weightLast * lastValue + weightNext * (*posValue);

        return result;
    }

};
}

#endif
