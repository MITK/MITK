/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkConvertToConcentrationRelativeFunctor_h
#define mitkConvertToConcentrationRelativeFunctor_h

#include <MitkPharmacokineticsExports.h>

namespace mitk {

    /** \class ConvertToConcentrationRelativeFunctor
     * \brief Binary functor that converts MR signal to concentration using relative signal enhancement.
     *
     * Computes concentration as the scaled relative difference between the current signal
     * and the baseline:
     * \code
     *   C(t) = k * (S(t) - S0) / S0
     * \endcode
     * where k is a proportionality factor, S(t) is the current signal, and S0 is the baseline.
     * Returns 0 if the baseline is zero.
     *
     * \tparam TInputPixel1 Pixel type of the dynamic signal image.
     * \tparam TInputPixel2 Pixel type of the baseline image.
     * \tparam TOutputpixel Pixel type of the output concentration image.
     * \sa ConcentrationCurveGenerator, ConvertToConcentrationAbsoluteFunctor
     */
    template <class TInputPixel1, class TInputPixel2, class TOutputpixel>
    class MITKPHARMACOKINETICS_EXPORT ConvertToConcentrationRelativeFunctor
    {

    public:
        ConvertToConcentrationRelativeFunctor(): m_k(0.0) {} ;
        ~ConvertToConcentrationRelativeFunctor() {};

        /** \brief Initializes the functor with the scaling factor.
         *  \param[in] factor The proportionality factor (k). */
        void initialize(double factor)
        {

            m_k = factor;
        }

        /** \brief Inequality comparison operator. */
        bool operator!=( const ConvertToConcentrationRelativeFunctor & other)const
        {
            return !(*this == other);
        }
        /** \brief Equality comparison operator. */
        bool operator==( const ConvertToConcentrationRelativeFunctor & other) const
        {
            return (this->m_k == other.m_k);
        }

        /** \brief Computes concentration from signal and baseline pixel values.
         *  \param[in] value The current signal intensity S(t).
         *  \param[in] baseline The baseline signal intensity S0.
         *  \return The computed concentration. Returns 0 if baseline is zero. */
        inline TOutputpixel operator()( const TInputPixel1 & value, const TInputPixel2 & baseline)
        {
            TOutputpixel concentration(0);

            if(baseline != 0)
            {
                concentration = this->m_k * (double)(value- baseline)/baseline ;
            }

            return concentration;
        }

    private:
        double m_k;

    };

}
#endif
