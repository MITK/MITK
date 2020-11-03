/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKCONVERTT2CONCENTRATIONFUNCTOR
#define MITKCONVERTT2CONCENTRATIONFUNCTOR

#include "MitkPharmacokineticsExports.h"

namespace mitk {

    template <class TInputPixel1, class TInputPixel2, class TOutputpixel>
    class MITKPHARMACOKINETICS_EXPORT ConvertT2ConcentrationFunctor
    {

    public:
        ConvertT2ConcentrationFunctor(): m_k(0.0), m_TE(0.0) {} ;
        ~ConvertT2ConcentrationFunctor() {};

        void initialize(double factor, double TE)
        {

            this->m_k = factor;
            this->m_TE = TE;
        }

        bool operator!=( const ConvertT2ConcentrationFunctor & other)const
        {
            return !(*this == other);
        }
        bool operator==( const ConvertT2ConcentrationFunctor & other) const
        {
            return (this->m_k == other.m_k && this->m_TE == other.m_TE);
        }

        inline TOutputpixel operator()( const TInputPixel1 & value, const TInputPixel2 & baseline)
        {
            double concentration = 0.0;
            if(value !=0 && baseline != 0)
            {
              concentration = (-1.) * (this->m_k / this->m_TE) * log(static_cast<double>(value) / baseline);

            }
            return static_cast<TOutputpixel>(concentration);
        }

    private:
        double m_k;
        double m_TE;

    };

}
#endif // MITKCONVERTT2CONCENTRATIONFUNCTOR

