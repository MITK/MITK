/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#ifndef MITKCOLORSEQUENCERAINBOW_H_URTESEINDEUTIGEKENNUNG_02
#define MITKCOLORSEQUENCERAINBOW_H_URTESEINDEUTIGEKENNUNG_02

#include "mitkColorSequence.h"
#include "MitkDataTypesExtExports.h"

namespace mitk
{

class MITKDATATYPESEXT_EXPORT ColorSequenceRainbow : public ColorSequence
{
public:
  ColorSequenceRainbow();
  virtual ~ColorSequenceRainbow();

  /*!
  \brief method to return another color
  */
  virtual Color GetNextColor();

  /*!
  \brief method to set the color-index to begin again
  */
  virtual void GoToBegin();

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
