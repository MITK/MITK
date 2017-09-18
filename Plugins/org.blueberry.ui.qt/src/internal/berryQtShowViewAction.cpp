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

#include <AutoplanLogging.h>

namespace berry
{

// TODO ism temporary translate
// it is hard to find point, where plugin name is read from file

const QMap<QString, QString> QtShowViewAction::plugingTranslateNames = { 
  { "AICP-Registration", QAction::tr("AICP-Registration") },
  { "Automatic Segmentation", QAction::tr("Automatic Segmentation")},
  { "BasicImageProcessing", QAction::tr("BasicImageProcessing") },
  { "Clipping Plane", QAction::tr("Clipping Plane") },
  { "Display", QAction::tr("Display") },
  { "Dicom Inspector", QAction::tr("Dicom Inspector") },
  { "Data Manager", QAction::tr("Data Manager") },
  { "EditSegmentation", QAction::tr("EditSegmentation") },
  { "Emphysema Diagnostics", QAction::tr("Emphysema Diagnostics") },
  { "Geometry Tools", QAction::tr("Geometry Tools") },
  { "Hepar Couinaud Segmentation", QAction::tr("Hepar Couinaud Segmentation") },
  { "IGT EchoTrack", QAction::tr("IGT EchoTrack") },
  { "IGT EchoTrack Calibration", QAction::tr("IGT EchoTrack Calibration") },
  { "Image Denoising", QAction::tr("Image Denoising") },
  { "Image Navigator", QAction::tr("Image Navigator") },
  { "Image Resize", QAction::tr("Image Resize") },
  { "Image Local Statistics", QAction::tr("Image Local Statistics") },
  { "Incremental segmentation view", QAction::tr("Incremental segmentation view") },
  { "LiverResectionView", QAction::tr("LiverResectionView") },
  { "DiffusionADC", QAction::tr("DiffusionADC") },
  { "Perfusion", QAction::tr("Perfusion") },
  { "LiverSegmentation", QAction::tr("LiverSegmentation") },
  { "Lung Trauma", QAction::tr("Lung Trauma") },
  { "Measurement", QAction::tr("Measurement") },
  { "Object Browser", QAction::tr("Object Browser") },
  { "OpenIGTLink Manager", QAction::tr("OpenIGTLink Manager") },
  { "Overlay Manager", QAction::tr("Overlay Manager") },
  { "RegionGrowing Segmentation", QAction::tr("RegionGrowing Segmentation") },
  { "PointSet Interaction", QAction::tr("PointSet Interaction") },
  { "PointBasedRegistration", QAction::tr("PointBasedRegistration") },
  { "PointSet Interaction Ext", QAction::tr("PointSet Interaction Ext") },
  { "Point Set Interaction Multispectrum", QAction::tr("Point Set Interaction Multispectrum") },
  { "Properties", QAction::tr("Properties") },
  { "Remeshing", QAction::tr("Remeshing") },
  { "RigidRegistration", QAction::tr("RigidRegistration") },
  { "RT Dose Visualization", QAction::tr("RT Dose Visualization") },
  { "Segmentation", QAction::tr("Segmentation") },
  { "Segmentation Metrics", QAction::tr("Segmentation Metrics") },
  { "Segmentation Comparator", QAction::tr("Segmentation Comparator") },
  { "Segmentation Lungs", QAction::tr("Segmentation Lungs") },
  { "Segmentation Utilities", QAction::tr("Segmentation Utilities") },
  { "Statistics", QAction::tr("Statistics") },
  { "ThoraxView", QAction::tr("ThoraxView") },
  { "Tracking", QAction::tr("Tracking") },
  { "Tube Graph", QAction::tr("Tube Graph") },
  { "TumorSegmentation", QAction::tr("TumorSegmentation") },
  { "Vascular Structure Segmentation", QAction::tr("Vascular Structure Segmentation") },
  { "Vascular Segmentation", QAction::tr("Vascular Segmentation") },
  { "Vessels Enhancing", QAction::tr("Vessels Enhancing") },
  { "View Navigator", QAction::tr("View Navigator") },
  { "Volume Visualization", QAction::tr("Volume Visualization") },
  { "Curved Planar Reconstruction", QAction::tr("Curved Planar Reconstruction") },
};


QtShowViewAction::QtShowViewAction(IWorkbenchWindow::Pointer window,
    IViewDescriptor::Pointer desc) :
  QAction(nullptr)
{
  this->setParent(static_cast<QWidget*>(window->GetShell()->GetControl()));

  QString label = desc->GetLabel();
  if (plugingTranslateNames.contains(label)) {
    this->setText(plugingTranslateNames[label]);
  } else {
    AUTOPLAN_FATAL << ("No name provided for plugin: " + label).toStdString();
    this->setText("NO_PLUGIN_NAME");
  }

  this->setToolTip(QString(desc->GetLabel().toUtf8().constData()));

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
