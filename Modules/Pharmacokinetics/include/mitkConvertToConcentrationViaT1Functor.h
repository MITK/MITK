/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkConvertToConcentrationViaT1Functor_h
#define mitkConvertToConcentrationViaT1Functor_h

#include "itkMath.h"

#include "MitkPharmacokineticsExports.h"

namespace mitk {

    template <class TInputPixel1, class TInputPixel2, class TInputPixel3, class TOutputpixel>
    class MITKPHARMACOKINETICS_EXPORT ConvertToConcentrationViaT1CalcFunctor
    {

    public:
      ConvertToConcentrationViaT1CalcFunctor(): m_relaxivity(0.0), m_TR(0.0),  m_flipangle(0.0), m_flipanglePDW(0.0) {};
      ~ConvertToConcentrationViaT1CalcFunctor() {};

        void initialize(double relaxivity, double TR, double flipangle, double flipanglePDW)
        {
          m_relaxivity = relaxivity;
          m_TR = TR;
          m_flipangle = flipangle;
          m_flipanglePDW = flipanglePDW;
        }

        bool operator!=( const ConvertToConcentrationViaT1CalcFunctor & other) const
        {
            return !(*this == other);

        }
       bool operator==( const ConvertToConcentrationViaT1CalcFunctor & other) const
        {
           return (this->m_relaxivity == other.m_relaxivity) && (this->m_TR == other.m_TR) && (this->m_flipangle == other.m_flipangle) && (this->m_flipanglePDW == other.m_flipanglePDW);
        }


        inline TOutputpixel operator()( const TInputPixel1 & value, const TInputPixel2 & baseline, const TInputPixel3 & pdw)
        {
          TOutputpixel concentration(0.0);
          if (baseline != 0 && pdw != 0 && value != 0)
          {
            // calculate signal scaling factor S0 and pre-contrast T1 relaxation time T10
            const double a = pdw * sin(m_flipangle) / (baseline * sin(m_flipanglePDW));
            const double b = (a - 1.0) / (a * cos(m_flipanglePDW) - cos(m_flipangle));
            const double T10 = static_cast<double>((-1.0) * m_TR / log(b));
            const double lambda = exp((-1.0) * m_TR / T10);
            const double S0 = pdw * (1.0-lambda * cos(m_flipanglePDW)) / ((1.0 - lambda) * sin(m_flipanglePDW));

            // calculate T1
            const double c = (value - S0 * sin(m_flipangle)) / (value * cos(m_flipangle) - S0 * sin(m_flipangle));
            const double T1 = static_cast<double>((-1.0) * m_TR / log(c));

            //calculate concentration
            concentration = static_cast<double>((1.0 / T1 - 1.0 / T10) / (m_relaxivity/1000.0));
          }
          else
          {
            concentration = 0.0;
          }

          return concentration;
        }
    private:
      double m_relaxivity;
      double m_TR;
      double m_flipangle;
      double m_flipanglePDW;
    };

}
#endif
