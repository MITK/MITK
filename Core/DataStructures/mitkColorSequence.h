/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef MITKCOLORSEQUENCE_H_URTESEINDEUTIGEKENNUNG_01
#define MITKCOLORSEQUENCE_H_URTESEINDEUTIGEKENNUNG_01

#include <mitkColorProperty.h>

namespace mitk
{

/*!
  \brief Inferface for creating a sequence of nice/matching/appropriate/... colors.

  See derived classes for implemented sequences. 
*/
class MITK_CORE_EXPORT ColorSequence
{
public:
  ColorSequence();
  virtual ~ColorSequence();

  /*!  
  \brief Return another color
  */ 
  virtual Color GetNextColor() = 0; 

  /*!  
  \brief Set the color-index to begin again
  */ 
  virtual void GoToBegin() = 0;

};

}

#endif


