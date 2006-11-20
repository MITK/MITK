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


#include "mitkLightBoxResultImageWriter.h"

#include <mitkImage.h>
#include <mitkImageSliceSelector.h>
#include <mitkFrameOfReferenceUIDManager.h>
#include <mitkChiliPlugin.h>

#include <itkRootTreeIterator.h>
#include <itkImageFileReader.h>


mitk::LightBoxResultImageWriter::LightBoxResultImageWriter()
{
}

mitk::LightBoxResultImageWriter::~LightBoxResultImageWriter()
{
}

const mitk::Image *mitk::LightBoxResultImageWriter::GetInput(void)
{
  return NULL;
}

void mitk::LightBoxResultImageWriter::SetInput(const mitk::Image *)
{
}

void mitk::LightBoxResultImageWriter::SetInputByNode(const mitk::DataTreeNode *)
{
}

const mitk::Image *mitk::LightBoxResultImageWriter::GetSourceImage(void)
{
  return NULL;
}

/// set the image that should be stored to the database
void mitk::LightBoxResultImageWriter::SetSourceImage(const mitk::Image *)
{
}

/// set the "example image" that is needed for writing (and already existent in the database)
bool mitk::LightBoxResultImageWriter::SetSourceByTreeSearch(mitk::DataTreeIteratorBase* )
{
  return false;
}
void mitk::LightBoxResultImageWriter::SetLightBox(QcLightbox* )
{
}

void mitk::LightBoxResultImageWriter::SetLightBoxToCurrentLightBox()
{
}

bool mitk::LightBoxResultImageWriter::SetLightBoxToNewLightBox()
{
  return false;
}

bool mitk::LightBoxResultImageWriter::SetLightBoxToCorrespondingLightBox()
{
 return false;
}

QcLightbox* mitk::LightBoxResultImageWriter::GetLightBox() const
{
  return NULL;
}


void mitk::LightBoxResultImageWriter::Write() const
{
}
