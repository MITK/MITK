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


#ifndef MITKTRANFERFUNCTIONPROPERTY_H_HEADER_INCLUDED
#define MITKTRANFERFUNCTIONPROPERTY_H_HEADER_INCLUDED

#include "mitkGenericProperty.h"
#include "mitkTransferFunction.h"

namespace mitk {
   
   typedef GenericProperty<mitk::TransferFunction::Pointer> TransferFunctionProperty;
} // namespace mitk  

#endif /* MITKTRANFERFUNCTIONPROPERTY_H_HEADER_INCLUDED */ 
