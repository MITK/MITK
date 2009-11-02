/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 19649 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef QmitkSegmentationPerspective_h_includ
#define QmitkSegmentationPerspective_h_includ

#include <cherryIPerspectiveFactory.h>

struct QmitkSegmentationPerspective : public cherry::IPerspectiveFactory
{

  void CreateInitialLayout(cherry::IPageLayout::Pointer layout);

};

#endif

