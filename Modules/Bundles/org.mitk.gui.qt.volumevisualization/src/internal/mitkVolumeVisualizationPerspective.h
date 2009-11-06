/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2009-11-02 20:38:30 +0100 (Mo, 02 Nov 2009) $
 Version:   $Revision: 19649 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef mitkVolumeVisualizationPerspective_h_includ
#define mitkVolumeVisualizationPerspective_h_includ

#include <cherryIPerspectiveFactory.h>

struct mitkVolumeVisualizationPerspective : public cherry::IPerspectiveFactory
{

  void CreateInitialLayout(cherry::IPageLayout::Pointer layout);

};

#endif

