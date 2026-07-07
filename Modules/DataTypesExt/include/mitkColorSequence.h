/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkColorSequence_h
#define mitkColorSequence_h

#include <MitkDataTypesExtExports.h>
#include <mitkColorProperty.h>

namespace mitk
{
  /**
   * \brief Abstract interface for generating a sequence of distinguishable colors.
   *
   * Subclasses implement specific color sequences (e.g., cycling through hue
   * values or using a predefined rainbow palette).
   *
   * \sa ColorSequenceCycleH, ColorSequenceRainbow
   */
  class MITKDATATYPESEXT_EXPORT ColorSequence
  {
  public:
    /** \brief Default constructor. */
    ColorSequence();

    /** \brief Virtual destructor. */
    virtual ~ColorSequence();

    /**
     * \brief Return the next color in the sequence.
     *
     * \return The next mitk::Color value.
     */
    virtual Color GetNextColor() = 0;

    /**
     * \brief Reset the sequence to the beginning.
     */
    virtual void GoToBegin() = 0;
  };
}

#endif
