/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef MITKCOLORSEQUENCERAINBOW_H_URTESEINDEUTIGEKENNUNG_02
#define MITKCOLORSEQUENCERAINBOW_H_URTESEINDEUTIGEKENNUNG_02

#include "MitkDataTypesExtExports.h"
#include "mitkColorSequence.h"

namespace mitk
{
  class MITKDATATYPESEXT_EXPORT ColorSequenceRainbow : public ColorSequence
  {
  public:
    ColorSequenceRainbow();
    ~ColorSequenceRainbow() override;

    /*!
    \brief method to return another color
    */
    Color GetNextColor() override;

    /*!
    \brief method to set the color-index to begin again
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
