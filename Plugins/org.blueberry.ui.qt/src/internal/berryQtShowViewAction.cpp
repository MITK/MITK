/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryQtShowViewAction.h"

#include <berryIWorkbenchPage.h>
#include <berryUIException.h>

namespace berry
{

// TODO ism temporary translate
// it is hard to find point, where plugin name is read from file

const QString QtShowViewAction::plugingTranslateNames[] = { 
  QAction::tr("BasicImageProcessing"),   
  QAction::tr("Clipping Plane"),
  QAction::tr("Display"),
  QAction::tr("Data Manager"),
  QAction::tr("Image Navigator"),
  QAction::tr("LiverSegmentation"),
  QAction::tr("Measurement"),
  QAction::tr("PointSet Interaction"),
  QAction::tr("RegionGrowing Segmentation"),
  QAction::tr("Segmentation"),
  QAction::tr("Segmentation Utilities"),
  QAction::tr("Statistics"),
  QAction::tr("Vascular Structure Segmentation"),
  QAction::tr("Volume Visualization")
};


QtShowViewAction::QtShowViewAction(IWorkbenchWindow::Pointer window,
    IViewDescriptor::Pointer desc) :
  QAction(nullptr)
{
  plugingSrcNames << "BasicImageProcessing" << "Clipping Plane" << "Display" << "Data Manager" << "Image Navigator" << "LiverSegmentation" <<
    "Measurement" << "PointSet Interaction" << "RegionGrowing Segmentation" << "Segmentation" << "Segmentation Utilities" <<
    "Statistics" << "Vascular Structure Segmentation" << "Volume Visualization";

  this->setParent(static_cast<QWidget*>(window->GetShell()->GetControl()));

  bool findTranslate = false;
  for (int i=0; i<plugingSrcNames.size(); i++) {
    if (plugingSrcNames[i].compare(QString(desc->GetLabel().toUtf8().constData())) == 0) {
      this->setText(plugingTranslateNames[i]);
      this->setToolTip(plugingTranslateNames[i]);    
      findTranslate = true;
      break;
    }
  }
  if (!findTranslate) {
    this->setToolTip(QString(desc->GetLabel().toUtf8().constData()));
    this->setText(QString(desc->GetLabel().toUtf8().constData()));
  }
  this->setIconVisibleInMenu(true);

  QIcon icon = desc->GetImageDescriptor();
  this->setIcon(icon);

  m_Window = window.GetPointer();
  m_Desc = desc;

  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}

void QtShowViewAction::Run()
{
  IWorkbenchPage::Pointer page = m_Window->GetActivePage();
  if (page.IsNotNull())
  {
    try
    {
      page->ShowView(m_Desc->GetId());
    }
    catch (const PartInitException& e)
    {
      BERRY_ERROR << "Error: " << e.what();
    }
  }
}

}
