/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkConvertToConcentrationAbsoluteFunctor_h
#define mitkConvertToConcentrationAbsoluteFunctor_h

#include <MitkPharmacokineticsExports.h>

namespace mitk {

    /** \class ConvertToConcentrationAbsoluteFunctor
     * \brief Binary functor that converts MR signal to concentration using absolute signal enhancement.
     *
     * Computes concentration as the scaled absolute difference between the current signal
     * and the baseline:
     * \code
     *   C(t) = k * (S(t) - S0)
     * \endcode
     * where k is a proportionality factor, S(t) is the current signal, and S0 is the baseline.
     *
     * \tparam TInputPixel1 Pixel type of the dynamic signal image.
     * \tparam TInputPixel2 Pixel type of the baseline image.
     * \tparam TOutputpixel Pixel type of the output concentration image.
     * \sa ConcentrationCurveGenerator, ConvertToConcentrationRelativeFunctor
     */
    template <class TInputPixel1, class TInputPixel2, class TOutputpixel>
    class MITKPHARMACOKINETICS_EXPORT ConvertToConcentrationAbsoluteFunctor
    {

    public:
        ConvertToConcentrationAbsoluteFunctor(): m_k(0.0) {} ;
        ~ConvertToConcentrationAbsoluteFunctor() {};

        /** \brief Initializes the functor with the scaling factor.
         *  \param[in] factor The proportionality factor (k). */
        void initialize(double factor)
        {

            m_k = factor;
        }

        /** \brief Inequality comparison operator. */
        bool operator!=( const ConvertToConcentrationAbsoluteFunctor & other)const
        {
            return !(*this == other);
        }
        /** \brief Equality comparison operator. */
        bool operator==( const ConvertToConcentrationAbsoluteFunctor & other) const
        {
            return (this->m_k == other.m_k);
        }

        /** \brief Computes concentration from signal and baseline pixel values.
         *  \param[in] value The current signal intensity S(t).
         *  \param[in] baseline The baseline signal intensity S0.
         *  \return The computed contrast agent concentration as k * (value - baseline). */
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
