/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKCONVERTTOCONCENTRATIONTURBOFLASCHFUNCTOR_H
#define MITKCONVERTTOCONCENTRATIONTURBOFLASCHFUNCTOR_H

#include "MitkPharmacokineticsExports.h"

namespace mitk {

    template <class TInputPixel1, class TInputPixel2, class TOutputpixel>
    class MITKPHARMACOKINETICS_EXPORT ConvertToConcentrationTurboFlashFunctor
    {

    public:
        ConvertToConcentrationTurboFlashFunctor() : m_Trec(0), m_alpha(0), m_T10(0) {};
        ~ConvertToConcentrationTurboFlashFunctor() {};

        void initialize(double relaxationtime, double relaxivity, double recoverytime)
        {
            m_Trec = relaxationtime;
            m_alpha = relaxivity;
            m_T10 = recoverytime;
        }

        bool operator!=( const ConvertToConcentrationTurboFlashFunctor & other)const
        {
            return !(*this == other);
        }
        bool operator==( const ConvertToConcentrationTurboFlashFunctor & other) const
        {
            return (this->m_Trec == other.m_Trec) && (this->m_alpha == other.m_alpha) && (this->m_T10 == other.m_T10);
        }

        inline TOutputpixel operator()( const TInputPixel1 & value, const TInputPixel2 & baseline)
        {
            TOutputpixel concentration(0);


            //Only for TurboFLASH sequencen
            if (baseline != 0 && ((double)value/baseline - exp(m_Trec/m_T10) * ((double)value/baseline - 1)) > 0 )
            {
                concentration = -1 / (m_Trec * m_alpha) * log((double)value/baseline - exp(m_Trec/m_T10) * ((double)value/baseline - 1));
            }


            return concentration;
        }

    private:
        double m_Trec;
        double m_alpha;
        double m_T10;
    };

}

#endif // MITKCONVERTTOCONCENTRATIONTURBOFLASCHFUNCTOR_H
