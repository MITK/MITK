#ifndef mitkHalfLifeConstants_h
#define mitkHalfLifeConstants_h

#include <string>
#include <MitkPETExports.h>

namespace mitk
{

/**
 * \brief Physical half-life constants for common PET radionuclides.
 *
 * Provides the display names and half-life values (in seconds) for radionuclides
 * commonly used in Positron Emission Tomography (PET) imaging. These constants
 * are used for SUV (Standardized Uptake Value) calculations involving radioactive
 * decay correction.
 *
 * \sa computeSUVbw, computeSUVbwScaleFactor, SUVbwFunctorPolicy
 */
struct MITKPET_EXPORT HALFLIFECONSTANTS
{
    static const std::string NAME_18F;    ///< \brief Display name for Fluorine-18 ("18F").
    static const double VALUE_18F;        ///< \brief Half-life of Fluorine-18 in seconds (6586.26 s, approx. 109.77 min).

    static const std::string NAME_68Ga;   ///< \brief Display name for Gallium-68 ("68Ga").
    static const double VALUE_68Ga;       ///< \brief Half-life of Gallium-68 in seconds (4057.74 s, approx. 67.63 min).

    static const std::string NAME_11C;    ///< \brief Display name for Carbon-11 ("11C").
    static const double VALUE_11C;        ///< \brief Half-life of Carbon-11 in seconds (1223.4 s, approx. 20.39 min).

    static const std::string NAME_15O;    ///< \brief Display name for Oxygen-15 ("15O").
    static const double VALUE_15O;        ///< \brief Half-life of Oxygen-15 in seconds (122.24 s, approx. 2.04 min).
};

}

#endif // HALFLIFECONSTANTS_H
