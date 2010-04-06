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

#include "Step6RegionGrowing.txx"

/// Instantiate the access function for 2D and all datatypes.
/// Instantiation for 3D is done in Step6RegionGrowing2.
/// Some compilers have memory problems without the explicit 
/// instantiation, some even need the separation in 2D/3D.
InstantiateAccessFunctionForFixedDimension_1(RegionGrowing, 2, Step6*) 

/**
\example Step6RegionGrowing1.cpp
*/
