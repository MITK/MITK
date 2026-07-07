/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkConvertToConcentrationTurboFlashFunctor_h
#define mitkConvertToConcentrationTurboFlashFunctor_h

#include <MitkPharmacokineticsExports.h>

namespace mitk {

    /** \class ConvertToConcentrationTurboFlashFunctor
     * \brief Binary functor that converts TurboFLASH MR signal values to contrast agent concentration.
     *
     * Uses the TurboFLASH-specific formula to compute concentration:
     * \code
     *   C(t) = -1 / (Trec * alpha) * ln( S(t)/S0 - exp(Trec/T10) * (S(t)/S0 - 1) )
     * \endcode
     * where Trec is the recovery time, alpha is the relaxivity, T10 is the pre-contrast
     * T1 relaxation time, S(t) is the current signal, and S0 is the baseline signal.
     *
     * \tparam TInputPixel1 Pixel type of the dynamic signal image.
     * \tparam TInputPixel2 Pixel type of the baseline image.
     * \tparam TOutputpixel Pixel type of the output concentration image.
     * \sa ConcentrationCurveGenerator
     */
    template <class TInputPixel1, class TInputPixel2, class TOutputpixel>
    class MITKPHARMACOKINETICS_EXPORT ConvertToConcentrationTurboFlashFunctor
    {

    private:
      double m_Trec;
      double m_alpha;
      double m_T10;

    public:
        ConvertToConcentrationTurboFlashFunctor() : m_Trec(0), m_alpha(0), m_T10(0) {};
        ~ConvertToConcentrationTurboFlashFunctor() {};

        /** \brief Initializes the functor with TurboFLASH sequence parameters.
         *  \param[in] relaxationtime The pre-contrast T1 relaxation time (T10).
         *  \param[in] relaxivity The contrast agent relaxivity (alpha).
         *  \param[in] recoverytime The recovery time (Trec). */
        void initialize(double relaxationtime, double relaxivity, double recoverytime)
        {
            m_Trec = recoverytime;
            m_alpha = relaxivity;
            m_T10 = relaxationtime;
        }

        /** \brief Inequality comparison operator. */
        bool operator!=( const ConvertToConcentrationTurboFlashFunctor & other)const
        {
            return !(*this == other);
        }
        /** \brief Equality comparison operator. */
        bool operator==( const ConvertToConcentrationTurboFlashFunctor & other) const
        {
            return (this->m_Trec == other.m_Trec) && (this->m_alpha == other.m_alpha) && (this->m_T10 == other.m_T10);
        }

        /** \brief Computes concentration from signal and baseline pixel values.
         *
         * Returns 0 if the baseline is zero or if the logarithm argument is non-positive.
         * \param[in] value The current signal intensity S(t).
         * \param[in] baseline The baseline signal intensity S0.
         * \return The computed contrast agent concentration. */
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


    };

}

#endif
