/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef CAMERAVISUALIZATION_H_INCLUDED
#define CAMERAVISUALIZATION_H_INCLUDED

#include "mitkNavigationDataVisualizationFilter.h"

namespace mitk {
  class CameraVisualization : public NavigationDataVisualizationFilter
  {
  public:
    mitkClassMacro(CameraVisualization,NavigationDataVisualizationFilter);
    itkNewMacro(Self);
  protected:
    CameraVisualization();
    virtual ~CameraVisualization();

    /**Documentation
    * \brief filter execute method
    *
    * positions and orients camera according to NavigationData
    */
    virtual void GenerateData();
  };
} // namespace mitk


#endif