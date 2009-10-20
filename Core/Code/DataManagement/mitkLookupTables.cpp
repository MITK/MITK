/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:31:16 +0200 (Di, 12. Mai 2009) $
Version:   $Revision: 17175 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkLookupTables.h"

template class mitk::GenericLookupTable<bool>;
template class mitk::GenericLookupTable<float>;
template class mitk::GenericLookupTable<int>;
template class mitk::GenericLookupTable<std::string>;


mitkSpecializeGenericLookupTableOperator(mitk::BoolLookupTable);
mitkSpecializeGenericLookupTableOperator(mitk::FloatLookupTable);
mitkSpecializeGenericLookupTableOperator(mitk::IntLookupTable);
mitkSpecializeGenericLookupTableOperator(mitk::StringLookupTable);
