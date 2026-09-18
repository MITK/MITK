/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSUVInputModel_h
#define mitkSUVInputModel_h

#include <mitkSUVCalculationHelper.h>
#include <mitkSUVNormalizationStrategy.h>

#include <MitkPETExports.h>

namespace mitk
{
  class IPropertyProvider;

  /**
   * \brief Whether the raw PET pixel values represent activity
   *        concentration or a pre-computed SUV.
   *
   * The two regimes need fundamentally different math:
   *   - \c ActivityConcentration: the standard SUV formula applies; the
   *     pixel may carry an additional pre-multiplier (e.g. a Philips
   *     CNTS activity-scale factor) that is folded into the activity
   *     pre-multiplication.
   *   - \c PrenormalizedSUV: the pixel is already in SUV form; the
   *     pipeline only re-normalizes from the source variant to the
   *     target variant. No decay correction, dose, or half-life are
   *     consumed.
   */
  enum class SUVPixelSemantics
  {
    ActivityConcentration,
    PrenormalizedSUV
  };

  /**
   * \brief Classification of how a PET image's raw pixel values relate
   *        to the SUV pipeline.
   *
   * Produced by ClassifyPETInput() from the input image's DICOM
   * properties. SUVImageFilter consults this to decide between the
   * activity-to-SUV path and the pre-normalized re-scale path.
   *
   * \par Field semantics by \c semantics
   *
   * \li \c ActivityConcentration:
   *       \c activityScale is the multiplier applied to the raw pixel
   *       before the standard SUV formula. \c 1.0 for plain BQML;
   *       Philips CNTS + activity-scale carries the lifted
   *       (7053,1009) factor.
   *
   * \li \c PrenormalizedSUV:
   *       \c sourceVariant is the SUV variant the pixel already encodes
   *       (BW for GML and Philips CNTS+SUV-scale; BSA for CM2ML).
   *       \c prenormScale is the multiplier applied to the raw pixel
   *       before the source-to-target re-normalization. \c 1.0 for
   *       GML/CM2ML; Philips CNTS+SUV-scale carries the lifted
   *       (7053,1000) factor.
   */
  struct MITKPET_EXPORT SUVInputModel
  {
    SUVPixelSemantics semantics    = SUVPixelSemantics::ActivityConcentration;
    double            activityScale = 1.0;
    SUVVariant        sourceVariant = SUVVariant::BW;
    double            prenormScale  = 1.0;
  };

  /**
   * \brief Classify the input pixel semantics from DICOM properties.
   *
   * Reads (0054,1001) Units, (0054,1006) SUV Type, (0008,0070)
   * Manufacturer, and (for Philips CNTS inputs) the lifted Philips
   * private scale factors (\c mitk.pet.PhilipsSUVScale,
   * \c mitk.pet.PhilipsActivityScale) to decide which arm of the SUV
   * pipeline applies and, for pre-normalized inputs, what variant the
   * stored pixels represent.
   *
   * \param[in] provider Source of DICOM properties; typically the
   *                     filter's input image.
   * \param[in] policy   Currently informational. The classification
   *                     matrix is identical for both policies; the
   *                     parameter exists so future IBSI-SUV
   *                     adaptations on the input-units axis can hook
   *                     in without an API break.
   * \return The classification result.
   *
   * \pre \p provider is not null.
   *
   * \throw MissingDICOMPropertyException if (0054,1001) Units is absent.
   * \throw UnsupportedPETUnitsException if (0054,1001) holds an
   *        unrecognized value, or if the value is CNTS and the
   *        manufacturer is not Philips.
   * \throw MissingPhilipsPETScaleException if (0054,1001) is CNTS,
   *        manufacturer is Philips, but neither of the two Philips
   *        private scale factors is present.
   */
  SUVInputModel MITKPET_EXPORT ClassifyPETInput(const IPropertyProvider* provider,
                                                 DICOMReadPolicy          policy);

  /**
   * \brief Classify the input pixel semantics of an Enhanced PET object.
   *
   * An Enhanced PET Image Storage object carries none of the classic PET
   * attributes: no (0054,1001) Units, no (0028,1052/1053) Rescale. The unit
   * lives in the Measurement Units Code Sequence inside the Real World
   * Value Mapping Sequence inside the functional groups, and the rescale in
   * the Pixel Value Transformation Sequence beside it. This reads those and
   * produces the same \c SUVInputModel the classic classifier does, so the
   * rest of the pipeline is unchanged.
   *
   * Pixel values arrive already rescaled -- GDCM applies the slope and
   * intercept at read time, exactly as for classic PET -- so this names the
   * unit and never re-applies a scale.
   *
   * \warning MITK's DICOM reader attaches one property set per file, so a
   *          multi-frame object collapses each attribute to a single value.
   *          Where the per-frame rescale differs between frames, the
   *          collapsed value would be applied to the whole volume; this
   *          refuses instead. It also refuses when it can see fewer values
   *          than the object has frames, since a path that resolves to
   *          nothing would otherwise read as uniform.
   *
   * \param[in] provider Source of DICOM properties.
   * \param[in] policy   Currently informational, as for ClassifyPETInput.
   * \return The classification result.
   *
   * \pre \p provider is not null and is an Enhanced PET object
   *      (\c IsEnhancedPETInput).
   *
   * \throw MissingDICOMPropertyException if no usable unit can be found.
   * \throw UnsupportedPETUnitsException if the unit code is outside the
   *        set the pipeline converts.
   * \throw EnhancedPETPerFrameVariationException if the per-frame rescale
   *        varies, or if fewer per-frame values are visible than the object
   *        has frames.
   */
  SUVInputModel MITKPET_EXPORT ClassifyEnhancedPETInput(const IPropertyProvider* provider,
                                                        DICOMReadPolicy          policy);
}

#endif
