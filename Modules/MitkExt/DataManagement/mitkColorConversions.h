/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Id:        $Id$
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

namespace mitk {

  /// some conversion routines to convert between different color spaces
  namespace ColorConversions {
    
    /// convert a HSV color to RGB color, H from 0 to 360, all other parameters 0 to 1
    void Hsv2Rgb(float h, float s, float v, float& r, float& g, float& b);
    /// convert a RGB color to HSV color, rgb parameters from 0 to 1
    void Rgb2Hsv(float r, float g, float b, float &h, float &s, float &v);

  } // ColorConversion

} // mitk

