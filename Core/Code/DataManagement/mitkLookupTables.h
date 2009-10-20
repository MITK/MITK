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


#ifndef MITKLOOKUPTABLES_H_HEADER_INCLUDED
#define MITKLOOKUPTABLES_H_HEADER_INCLUDED

#include "mitkGenericLookupTable.h"

/**Documentation
* \brief specializations of GenericLookupTable
*
* This file contains specializations of mitk::GenericLookupTable 
* for bool, float, int and std::string lookuptables
* \WARN: you have to call the mitkSpecializeGenericLookupTableOperator macro
* in mitkLookupTables.cpp with each specialization to add an ostream << operator
* for that lookuptable specialization.
*/
namespace mitk {
   mitkSpecializeGenericLookupTable(BoolLookupTable, bool);
   mitkSpecializeGenericLookupTable(FloatLookupTable, float);
   mitkSpecializeGenericLookupTable(IntLookupTable, int);
   mitkSpecializeGenericLookupTable(StringLookupTable, std::string);
} // namespace mitk

#endif /* MITKLOOKUPTABLES_H_HEADER_INCLUDED*/
