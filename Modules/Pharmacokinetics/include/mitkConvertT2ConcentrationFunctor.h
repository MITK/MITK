/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkConvertT2ConcentrationFunctor_h
#define mitkConvertT2ConcentrationFunctor_h

#include <MitkPharmacokineticsExports.h>

namespace mitk {

    /** \class ConvertT2ConcentrationFunctor
     * \brief Binary functor that converts T2-weighted MR signal values to contrast agent concentration.
     *
     * Computes concentration from the logarithmic ratio of the signal to the baseline:
     * \code
     *   C(t) = -(k / TE) * ln(S(t) / S0)
     * \endcode
     * where k is a proportionality factor, TE is the echo time, S(t) is the current signal,
     * and S0 is the baseline signal.
     *
     * \tparam TInputPixel1 Pixel type of the dynamic signal image.
     * \tparam TInputPixel2 Pixel type of the baseline image.
     * \tparam TOutputpixel Pixel type of the output concentration image.
     * \sa ConcentrationCurveGenerator
     */
    template <class TInputPixel1, class TInputPixel2, class TOutputpixel>
    class MITKPHARMACOKINETICS_EXPORT ConvertT2ConcentrationFunctor
    {

    public:
        ConvertT2ConcentrationFunctor(): m_k(0.0), m_TE(0.0) {} ;
        ~ConvertT2ConcentrationFunctor() {};

        /** \brief Initializes the functor with the required MR sequence parameters.
         *  \param[in] factor The proportionality factor (k) for the T2* relaxivity.
         *  \param[in] TE The echo time in ms. */
        void initialize(double factor, double TE)
        {

            this->m_k = factor;
            this->m_TE = TE;
        }

        /** \brief Inequality comparison operator. */
        bool operator!=( const ConvertT2ConcentrationFunctor & other)const
        {
            return !(*this == other);
        }
        /** \brief Equality comparison operator. */
        bool operator==( const ConvertT2ConcentrationFunctor & other) const
        {
            return (this->m_k == other.m_k && this->m_TE == other.m_TE);
        }

        /** \brief Computes concentration from signal and baseline pixel values.
         *  \param[in] value The current signal intensity S(t).
         *  \param[in] baseline The baseline signal intensity S0.
         *  \return The computed contrast agent concentration. Returns 0 if either input is 0. */
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
#endif
