/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKCONVERTTOCONCENTRATIONVIAT1CALCFUNCTOR_H
#define MITKCONVERTTOCONCENTRATIONVIAT1CALCFUNCTOR_H

#include "itkMath.h"

#include "MitkPharmacokineticsExports.h"

namespace mitk {

    template <class TInputPixel1, class TInputPixel2, class TInputPixel3, class TOutputpixel>
    class MITKPHARMACOKINETICS_EXPORT ConvertToConcentrationViaT1CalcFunctor
    {

    public:
        ConvertToConcentrationViaT1CalcFunctor(): m_relaxivity(0.0), m_TR(0.0),  m_flipangle(0.0) {};
		~ConvertToConcentrationViaT1CalcFunctor() {};

        void initialize(double relaxivity, double TR, double flipangle)
        {

			m_relaxivity = relaxivity;
			m_TR = TR;
            m_flipangle = flipangle;
        }

        bool operator!=( const ConvertToConcentrationViaT1CalcFunctor & other) const
        {
            return !(*this == other);

        }
       bool operator==( const ConvertToConcentrationViaT1CalcFunctor & other) const
        {
           return (this->m_relaxivity == other.m_relaxivity) && (this->m_TR == other.m_TR) && (this->m_flipangle == other.m_flipangle);
        }


        inline TOutputpixel operator()( const TInputPixel1 & value, const TInputPixel2 & baseline, const TInputPixel3 & nativeT1)
        {
            TOutputpixel concentration(0);
            double R10, R1;
            if (baseline !=0 && nativeT1 != 0 && value != 0)
            {
                double s =  (double) value/baseline;
                R10 = (double) 1/nativeT1;
                double tmp1 = log(1-s + s*exp(-R10*m_TR) - exp(-R10*m_TR)* cos(m_flipangle));
                double tmp2 = (1-s*cos(m_flipangle) + s*exp(-R10*m_TR)*cos(m_flipangle) - exp(-R10*m_TR)* cos(m_flipangle));

                R1 = (double) -1/m_TR * tmp1/tmp2 ;

                concentration = (double) (R1 - R10)/ m_relaxivity;
            }
            else
            {
                concentration = 0;
            }

                return concentration;
        }
    private:
		double m_relaxivity;
		double m_TR;
        double m_flipangle;

    };

}
#endif // MITKCONVERTTOCONCENTRATIONVIAT1CALCFUNCTOR_H
