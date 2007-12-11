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


#include "mitkProperties.h"

template class mitk::GenericProperty<bool>;
template class mitk::GenericProperty<int>;
template class mitk::GenericProperty<float>;
template class mitk::GenericProperty<double>;
template class mitk::GenericProperty<mitk::Point3D>;
template class mitk::GenericProperty<itk::Point<int, 3> >;
template class mitk::GenericProperty<mitk::Point4D>;

               
