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
#ifndef MITKCOLORSEQUENCE_H_DEFG2390JKD
#define MITKCOLORSEQUENCE_H_DEFG2390JKD

#include <mitkColorSequence.h>

namespace mitk
{

/*!
  \brief Creates a list of around 36 different colors, where one is easily distinguished from the preceding one.

  The list of colors starts with a fully saturated, full valued red.
  After that the sequence is generated like this:
  
  - first cycle through fully saturated colors (increase hue)
  - then cycle through colors with halfed saturation (increase hue)
  - then cycle through colors with halfed value (increase hue)
  
  Finally repeat colors.
  
*/
class ColorSequenceCycleH : public ColorSequence
{
  public:
    
    ColorSequenceCycleH();
    
    virtual ~ColorSequenceCycleH();

    /*!  
    \brief Return another color
    */ 
    virtual Color GetNextColor(); 
  
    /*!  
    \brief Rewind to first color
    */ 
    virtual void GoToBegin(); 
    
  protected:
      
    float color_h; // current hue (0 .. 360)
    float color_s; // current saturation (0 .. 1)
    float color_v; // current value (0 .. 1)
    
    int color_cycle;

};

}

#endif

