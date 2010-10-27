/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 14645 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <Poco/ClassLibrary.h>

#include <mitkIContextMenuAction.h>
#include <berryIViewPart.h>

#include "src/internal/QmitkSegmentationView.h"
#include "src/QmitkSegmentationPreferencePage.h"
#include "src/internal/QmitkThresholdAction.h"
#include "src/internal/QmitkCreatePolygonModelAction.h"
// #include "src/internal/QmitkStatisticsAction.h"
#include "src/internal/QmitkAutocropAction.h"


POCO_BEGIN_NAMED_MANIFEST(berryIViewPart, berry::IViewPart)
  POCO_EXPORT_CLASS(QmitkSegmentationView)
POCO_END_MANIFEST

POCO_BEGIN_NAMED_MANIFEST(berryIPreferencePage, berry::IPreferencePage)
  POCO_EXPORT_CLASS(QmitkSegmentationPreferencePage)
POCO_END_MANIFEST

POCO_BEGIN_NAMED_MANIFEST(mitkIContextMenuAction,  mitk::IContextMenuAction)
  POCO_EXPORT_CLASS(QmitkThresholdAction)
  // POCO_EXPORT_CLASS(QmitkCreatePolygonModelAction)
  // POCO_EXPORT_CLASS(QmitkStatisticsAction)
  POCO_EXPORT_CLASS(QmitkAutocropAction)
POCO_END_MANIFEST
