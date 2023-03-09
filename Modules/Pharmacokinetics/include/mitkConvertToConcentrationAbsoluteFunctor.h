/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkConvertToConcentrationAbsoluteFunctor_h
#define mitkConvertToConcentrationAbsoluteFunctor_h

#include "MitkPharmacokineticsExports.h"

namespace mitk {

    template <class TInputPixel1, class TInputPixel2, class TOutputpixel>
    class MITKPHARMACOKINETICS_EXPORT ConvertToConcentrationAbsoluteFunctor
    {

    public:
        ConvertToConcentrationAbsoluteFunctor(): m_k(0.0) {} ;
        ~ConvertToConcentrationAbsoluteFunctor() {};

        void initialize(double factor)
        {

            m_k = factor;
        }

        bool operator!=( const ConvertToConcentrationAbsoluteFunctor & other)const
        {
            return !(*this == other);
        }
        bool operator==( const ConvertToConcentrationAbsoluteFunctor & other) const
        {
            return (this->m_k == other.m_k);
        }

        inline TOutputpixel operator()( const TInputPixel1 & value, const TInputPixel2 & baseline)
        {
            TOutputpixel concentration(0);

                concentration = this->m_k * (double)(value- baseline) ;

            return concentration;
        }

    private:
        double m_k;

    };

}
#endif
