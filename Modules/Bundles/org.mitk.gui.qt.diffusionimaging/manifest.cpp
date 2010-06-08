/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Do, 28 Mai 2009) $
Version:   $Revision: 17495 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <Poco/ClassLibrary.h>

#include "src/internal/QmitkQBallReconstructionView.h"
#include "src/internal/QmitkDiffusionDicomImportView.h"
#include "src/internal/QmitkDiffusionQuantificationView.h"
#include "src/internal/QmitkTensorReconstructionView.h"
#include "src/internal/QmitkControlVisualizationPropertiesView.h"
//#include "src/internal/QmitkDiffusionImagingPerspective.h"

//POCO_BEGIN_NAMED_MANIFEST(berryIPerspectiveFactory, berry::IPerspectiveFactory)
//  POCO_EXPORT_CLASS(QmitkDiffusionImagingPerspective)
//POCO_END_MANIFEST

POCO_BEGIN_NAMED_MANIFEST(berryIViewPart, berry::IViewPart)
  POCO_EXPORT_CLASS(QmitkQBallReconstructionView)
  POCO_EXPORT_CLASS(QmitkDiffusionDicomImport)
  POCO_EXPORT_CLASS(QmitkDiffusionQuantificationView)
  POCO_EXPORT_CLASS(QmitkTensorReconstructionView)
  POCO_EXPORT_CLASS(QmitkControlVisualizationPropertiesView)
POCO_END_MANIFEST
