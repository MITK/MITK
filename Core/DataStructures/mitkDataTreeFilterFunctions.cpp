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

#include <mitkDataTreeFilterFunctions.h>

namespace mitk
{

//------ Some common filter functions ----------------------------------------------------

/// default filter, lets everything except NULL pointers pass
bool IsDataTreeNode(DataTreeNode* node)
{
  return ( node!= 0 );
}
  
bool IsGoodDataTreeNode(DataTreeNode* node)
{
  return ( node!= 0 && node->GetData() );
}

/* example for other possible tests *//*
bool IsASegmentation(DataTreeNode* node)
{
  return (    node
          &&  node->GetData() 
          &&  dynamic_cast<Image*>( node->GetData() ) 
          &&  node->GetProperty("segmentation").IsNotNull() 
         );
  return true;
}
*/


} // namespace

