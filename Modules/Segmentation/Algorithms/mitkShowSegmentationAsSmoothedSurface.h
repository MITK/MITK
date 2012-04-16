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

#ifndef MITK_SHOW_SEGMENTATION_AS_SMOOTHED_SURFACE_H
#define MITK_SHOW_SEGMENTATION_AS_SMOOTHED_SURFACE_H

#include "mitkSegmentationSink.h"
#include <mitkSurface.h>

namespace mitk
{
  class Segmentation_EXPORT ShowSegmentationAsSmoothedSurface : public SegmentationSink
  {
  public:
    mitkClassMacro(ShowSegmentationAsSmoothedSurface, SegmentationSink)
    mitkAlgorithmNewMacro(ShowSegmentationAsSmoothedSurface)

  protected:
    void Initialize(const NonBlockingAlgorithm *other = NULL);
    bool ReadyToRun();
    bool ThreadedUpdateFunction();
    void ThreadedUpdateSuccessful();

  private:
    ShowSegmentationAsSmoothedSurface();
    ~ShowSegmentationAsSmoothedSurface();

    Surface::Pointer m_Surface;
  };
}

#endif
