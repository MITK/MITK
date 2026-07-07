/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkColorSequenceRainbow_h
#define mitkColorSequenceRainbow_h

#include <MitkDataTypesExtExports.h>
#include <mitkColorSequence.h>

namespace mitk
{
  /**
   * \brief Predefined rainbow color sequence with eight distinguishable colors.
   *
   * Cycles through: default orange, red, yellow, blue, green, magenta, cyan,
   * and orange. After reaching the end the sequence wraps around.
   *
   * \sa ColorSequence, ColorSequenceCycleH
   */
  class MITKDATATYPESEXT_EXPORT ColorSequenceRainbow : public ColorSequence
  {
  public:
    /** \brief Construct and initialize the predefined color list. */
    ColorSequenceRainbow();

    /** \brief Destructor. */
    ~ColorSequenceRainbow() override;

    /**
     * \brief Return the next color in the rainbow sequence.
     *
     * Advances the internal index and wraps around at the end of the list.
     *
     * \return The next mitk::Color.
     */
    Color GetNextColor() override;

    /**
     * \brief Reset the color index to the beginning.
     */
    void GoToBegin() override;

  private:
    /*!
    \brief method that fills the pre-defnied colors
    */
    void InitColorList();

    /*!
    \brief global color-object that holds the actual color and changes to the next one
    */
    mitk::Color m_Color;

    /*!
    \brief color-index to iterate through the colors
    */
    unsigned int m_ColorIndex;

    /*!
    \brief vector-List with all pre-defined colors
    */
    std::vector<Color> m_ColorList;
  };
}

#endif
