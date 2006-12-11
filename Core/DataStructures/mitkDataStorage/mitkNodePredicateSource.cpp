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

#include "mitkNodePredicateSource.h"

mitk::NodePredicateSource::NodePredicateSource(mitk::DataTreeNode& n, bool allsources)
: NodePredicateBase(), m_BaseNode(n), m_SearchAllSources(allsources)
{
}

mitk::NodePredicateSource::~NodePredicateSource()
{
}


bool mitk::NodePredicateSource::CheckNode(mitk::DataTreeNode* node) const
{
  if (!node)
    throw 1;  // Insert Exception Handling here

  // check, if any of the source objects of m_BaseNode are equal to node.
  if (m_SearchAllSources)
    ;   // also search recursivly in sources of sources
  else
    ;   // only search in direct sources

  return true;
}
