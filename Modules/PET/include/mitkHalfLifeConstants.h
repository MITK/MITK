#ifndef mitkHalfLifeConstants_h
#define mitkHalfLifeConstants_h

#include <string>
#include <MitkPETExports.h>

namespace mitk
{

/**
 * \brief Physical half-life constants for common PET radionuclides.
 *
 * Provides display names and half-life values (in seconds) for radionuclides
 * commonly used in Positron Emission Tomography (PET) imaging. These constants
 * are used for SUV (Standardized Uptake Value) calculations involving
 * radioactive-decay correction.
 *
 * \par Source
 *
 * Values are taken from the NNDC NuDat 3 nuclear data search
 * (https://www.nndc.bnl.gov/nudat3/), retrieved 2026-04-30. The values shown
 * below are the recommended half-lives evaluated from the cited Nuclear Data
 * Sheets entries; the bracketed quantities are the experimental uncertainties
 * from the same source. Where NNDC reports the half-life in minutes, the
 * value here is the minute value multiplied by 60 (no further rounding).
 *
 * \sa computeSUVbw, computeSUVbwScaleFactor, SUVbwFunctorPolicy
 */
struct MITKPET_EXPORT HALFLIFECONSTANTS
{
    /** \brief Display name for Fluorine-18 ("18F"). */
    static const std::string NAME_18F;
    /** \brief Half-life of Fluorine-18 in seconds.
     *
     *  6586.2 s = 109.77 min. NNDC NuDat 3 reports 109.77 +/- 0.05 min.
     */
    static const double VALUE_18F;

    /** \brief Display name for Gallium-68 ("68Ga"). */
    static const std::string NAME_68Ga;
    /** \brief Half-life of Gallium-68 in seconds.
     *
     *  4062.6 s = 67.71 min. NNDC NuDat 3 reports 67.71 +/- 0.08 min.
     */
    static const double VALUE_68Ga;

    /** \brief Display name for Carbon-11 ("11C"). */
    static const std::string NAME_11C;
    /** \brief Half-life of Carbon-11 in seconds.
     *
     *  1221.8 s = 20.363 min. NNDC NuDat 3 reports 1221.8 +/- 0.8 s.
     */
    static const double VALUE_11C;

    /** \brief Display name for Oxygen-15 ("15O"). */
    static const std::string NAME_15O;
    /** \brief Half-life of Oxygen-15 in seconds.
     *
     *  122.24 s = 2.0373 min. NNDC NuDat 3 reports 122.24 +/- 0.16 s.
     */
    static const double VALUE_15O;
};

}

#endif // HALFLIFECONSTANTS_H
