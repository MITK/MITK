/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-06-15 14:28:00 +0200 (Fr, 15 Jun 2007) $
Version:   $Revision: 10778 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkPicDescriptorToNode.h"

// constructor
mitk::PicDescriptorToNode::PicDescriptorToNode()
{
}

// destructor
mitk::PicDescriptorToNode::~PicDescriptorToNode()
{
}

// set-function
void mitk::PicDescriptorToNode::SetInput( std::list< ipPicDescriptor* > /*inputPicDescriptorList*/, std::string /*inputSeriesOID*/ )
{
}

// get-function
std::vector< mitk::DataTreeNode::Pointer > mitk::PicDescriptorToNode::GetOutput()
{
  std::vector< mitk::DataTreeNode::Pointer > emptyResult;
  emptyResult.clear();
  return emptyResult;
}

// the "main"-function
void mitk::PicDescriptorToNode::Update()
{
}

