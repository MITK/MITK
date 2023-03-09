/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTwoTissueCompartmentModelDifferentialEquations_h
#define mitkTwoTissueCompartmentModelDifferentialEquations_h
#include "mitkNumericTwoTissueCompartmentModel.h"

namespace mitk{
/** @class TwoTissueCompartmentModelDifferentialEquations
 * @brief Helper Class for NumericTwoTissueCompartment Model: Defines the differential equations (Mass Balance Equations) in the
 * 2-tissue-compartment model for dynamic PET data modeling.
 * The 2-Tissue Compartment model is defined via the mass balance equations
 * dC1(t)/dt =  K1*Ca(t) - (k2 + k3)*C1(t) + k4*C2(t)
 * dC2(t)/dt = k3*C1(t) - k4*C2(t)
 * CT(t) = C_a(t)*VB + (1-VB)*(C1(t)+C2(t)
 * where Ca(t) is the plasma concentration(aterial input function)
 * Boost ODEINT performs a stepwise numeric integration (e.g. via Runge-Kutta method) of the initial value problem
 * x' = dx/dt = f(x,t)
 * It needs an operator () (a functor) that calculates dx/dt = dxdt for a given x and t.
 * Parameters are K1,k2,k3,k4, VB and the time dependent Ca(t) =AIF, that is interpolated to the current step t
*/

class TwoTissueCompartmentModelDifferentialEquations
{
public:

    typedef std::vector< double > AIFType;

    /** @brief Functor for differential equation of Two Tissue Compartment Model
     * Takes current state x = x(t) and time t and calculates the corresponding dxdt = dx/dt
    */
    void operator() (const mitk::NumericTwoTissueCompartmentModel::state_type &x, mitk::NumericTwoTissueCompartmentModel::state_type &dxdt, const double t)
    {
        double Ca_t = InterpolateAIFToCurrentTimeStep(t);

        dxdt[0] = this->K1*Ca_t-(this->k2+this->k3)*x[0] + this->k4*x[1];
        dxdt[1] = this->k3*x[0] - this->k4*x[1];
    }

    TwoTissueCompartmentModelDifferentialEquations() : K1(0), k2(0), k3(0), k4(0), m_AIF(0), m_AIFTimeGrid(0)
    {
    }

    /** @brief Initialize class with parameters K1, k2, k3 and k4 that are free fit parameters*/
    void initialize(double k_1, double k_2, double k_3, double k_4)
    {
        this->K1 = k_1;
        this->k2 = k_2;
        this->k3 = k_3;
        this->k4 = k_4;
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

    double K1;
    double k2;
    double k3;
    double k4;

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
