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

#include "mitkLightBoxResultImageWriter.h"

mitk::LightBoxResultImageWriter::LightBoxResultImageWriter()
{
}

mitk::LightBoxResultImageWriter::~LightBoxResultImageWriter()
{
}

void mitk::LightBoxResultImageWriter::SetInput( const mitk::Image* )
{
}

void mitk::LightBoxResultImageWriter::SetInputByDataTreeNode( const mitk::DataTreeNode* )
{
}

void mitk::LightBoxResultImageWriter::SetLevelWindow( LevelWindow levelwindow )
{
}

void mitk::LightBoxResultImageWriter::SetSeriesDescription( const std::string& )
{
}

void mitk::LightBoxResultImageWriter::SetPropertyList( const mitk::PropertyList::Pointer )
{
}

void mitk::LightBoxResultImageWriter::SetLightBox( QcLightbox* )
{
}

void mitk::LightBoxResultImageWriter::SetLightBoxToCurrentLightBox()
{
}

bool mitk::LightBoxResultImageWriter::SetLightBoxToNewLightBox()
{
  return false;
}

void mitk::LightBoxResultImageWriter::Write()
{
}
