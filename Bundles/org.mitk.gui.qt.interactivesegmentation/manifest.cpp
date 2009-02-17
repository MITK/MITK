/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-06-25 16:22:13 +0200 (Mi, 25 Jun 2008) $
Version:   $Revision: 14645 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <Poco/ClassLibrary.h>

#include <cherryIViewPart.h>
#include "src/internal/QmitkInteractiveSegmentationView.h"
#include "src/QmitkToolGUI.h"
#include "src/internal/QmitkBinaryThresholdToolGUI.h"
#include "src/internal/QmitkCalculateGrayValueStatisticsToolGUI.h"
#include "src/internal/QmitkDrawPaintbrushToolGUI.h"
#include "src/internal/QmitkPaintbrushToolGUI.h"
#include "src/internal/QmitkErasePaintbrushToolGUI.h"


POCO_BEGIN_NAMED_MANIFEST(cherryIViewPart, cherry::IViewPart)
  POCO_EXPORT_CLASS(QmitkInteractiveSegmentationView)
POCO_END_MANIFEST

POCO_BEGIN_NAMED_MANIFEST(QmitkToolGUI, QmitkToolGUI)
  POCO_EXPORT_CLASS(QmitkBinaryThresholdToolGUI)
  POCO_EXPORT_CLASS(QmitkCalculateGrayValueStatisticsToolGUI)
  POCO_EXPORT_CLASS(QmitkDrawPaintbrushToolGUI)
  POCO_EXPORT_CLASS(QmitkPaintbrushToolGUI)
  POCO_EXPORT_CLASS(QmitkErasePaintbrushToolGUI)
POCO_END_MANIFEST
