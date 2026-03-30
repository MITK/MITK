/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkColorSequenceCycleH_h
#define mitkColorSequenceCycleH_h

#include <MitkDataTypesExtExports.h>
#include <mitkColorSequence.h>

namespace mitk
{
  /**
   * \brief Generates ~36 distinguishable colors by cycling through HSV space.
   *
   * Starting from fully saturated red (H=0), the sequence cycles through:
   * -# Fully saturated colors (hue increments of 60 degrees)
   * -# Colors with halved saturation
   * -# Colors with halved value
   *
   * After exhausting all combinations the colors repeat.
   *
   * \sa ColorSequence, ColorSequenceRainbow
   */
  class MITKDATATYPESEXT_EXPORT ColorSequenceCycleH : public ColorSequence
  {
  public:
    /** \brief Construct and initialize to the first color (red, H=0). */
    ColorSequenceCycleH();

    /** \brief Destructor. */
    ~ColorSequenceCycleH() override;

    /**
     * \brief Return the next color in the HSV cycle.
     *
     * \return The next mitk::Color.
     */
    Color GetNextColor() override;

    /**
     * \brief Rewind to the first color in the sequence.
     */
    void GoToBegin() override;

    /**
     * \brief Advance the hue by a given number of cycle steps.
     *
     * Each step corresponds to a 60-degree hue increment. Negative values
     * are clamped to 0.
     *
     * \note This does not change saturation or value (i.e., the color cycle).
     *       Use SetColorCycle() to change those.
     *
     * \param[in] steps Number of 60-degree hue increments.
     */
    virtual void ChangeHueValueByCycleSteps(int steps);

    /**
     * \brief Set the hue to an absolute value.
     *
     * Values below 0 are clamped to 0. The range is [0, 360).
     *
     * \param[in] number The absolute hue value.
     */
    virtual void ChangeHueValueByAbsoluteNumber(float number);

    /**
     * \brief Set the color cycle index (0--5).
     *
     * Controls the saturation/value combination used. Combine with
     * hue changes to select specific colors.
     *
     * \param[in] cycle The cycle index (0--5).
     */
    virtual void SetColorCycle(unsigned short cycle);

  protected:
    float color_h; // current hue (0 .. 360)
    float color_s; // current saturation (0 .. 1)
    float color_v; // current value (0 .. 1)

    unsigned short color_cycle;
  };
}

#endif
