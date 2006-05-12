/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

class ColorSequence
{
public:
  ColorSequence();
  virtual ~ColorSequence();

  /*!  
  \brief method to return another color
  */ 
  virtual Color GetNextColor() = 0; 

  /*!  
  \brief method to set the color-index to begin again
  */ 
  virtual void GoToBegin() = 0;

};

}

#endif