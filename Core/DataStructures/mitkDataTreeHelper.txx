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

#include <mitkImageToItkMultiplexer.h>
#include <mitkStringProperty.h>
#include <mitkLevelWindowProperty.h>
#include <mitkDataTreeNodeFactory.h>

namespace mitk 
{

namespace DataTreeHelper 

{

template < typename TImageType >
mitk::DataTreeNode::Pointer 
AddItkImageToDataTree(TImageType* itkImage, mitk::DataTreeIteratorBase* iterator, const char* name=NULL, bool replaceByName=false)


}

}
