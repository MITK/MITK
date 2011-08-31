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

#include "mitkShowSegmentationAsSmoothedSurface.h"

using namespace mitk;
using namespace std;

ShowSegmentationAsSmoothedSurface::ShowSegmentationAsSmoothedSurface()
{
}

ShowSegmentationAsSmoothedSurface::~ShowSegmentationAsSmoothedSurface()
{
}

void ShowSegmentationAsSmoothedSurface::Initialize(const NonBlockingAlgorithm *other)
{
  Superclass::Initialize(other);

  // TODO: Set up default parameters
}

bool ShowSegmentationAsSmoothedSurface::ReadyToRun()
{
  try
  {
    Image::Pointer image;
    GetPointerParameter("Input", image);

    return image.IsNotNull() && GetGroupNode();
  }
  catch (const invalid_argument &)
  {
    return false;
  }
}

bool ShowSegmentationAsSmoothedSurface::ThreadedUpdateFunction()
{
  // TODO

  return true;
}

void ShowSegmentationAsSmoothedSurface::ThreadedUpdateSuccessful()
{
  // TODO

  Superclass::ThreadedUpdateSuccessful();
}
