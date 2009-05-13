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
#ifndef MITKCOLORSEQUENCEHALFTONES_H_URTESEINDEUTIGEKENNUNG_03
#define MITKCOLORSEQUENCEHALFTONES_H_URTESEINDEUTIGEKENNUNG_03

#include "mitkColorSequence.h"

namespace mitk
{
  class MITKEXT_CORE_EXPORT ColorSequenceHalfTones : public ColorSequence
  {
    public:
    ColorSequenceHalfTones();
    virtual ~ColorSequenceHalfTones();
    
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
  Color m_Color;

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


