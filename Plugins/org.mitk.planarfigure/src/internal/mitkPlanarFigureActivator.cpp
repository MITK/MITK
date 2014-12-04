/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkPlanarFigureActivator.h"
#include "mitkPlanarFigureObjectFactory.h"

#include "QmitkNodeDescriptorManager.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateAnd.h"

#include <QtPlugin>

void
mitk::PlanarFigureActivator::start(ctkPluginContext* /*context*/)
{
  QmitkNodeDescriptorManager* descriptorManager = QmitkNodeDescriptorManager::GetInstance();

  // Adding "PlanarLine"
  mitk::NodePredicateDataType::Pointer isPlanarLine = mitk::NodePredicateDataType::New("PlanarLine");
  descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarLine"), QString(":/QtWidgetsExt/PlanarLine_48.png"), isPlanarLine, descriptorManager));

  // Adding "PlanarCircle"
  mitk::NodePredicateDataType::Pointer isPlanarCircle = mitk::NodePredicateDataType::New("PlanarCircle");
  descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarCircle"), QString(":/QtWidgetsExt/PlanarCircle_48.png"), isPlanarCircle, descriptorManager));

  // Adding "PlanarEllipse"
  mitk::NodePredicateDataType::Pointer isPlanarEllipse = mitk::NodePredicateDataType::New("PlanarEllipse");
  descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarEllipse"), QString(":/QtWidgetsExt/PlanarEllipse_48.png"), isPlanarEllipse, descriptorManager));

  // Adding "PlanarAngle"
  mitk::NodePredicateDataType::Pointer isPlanarAngle = mitk::NodePredicateDataType::New("PlanarAngle");
  descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarAngle"), QString(":/QtWidgetsExt/PlanarAngle_48.png"), isPlanarAngle, descriptorManager));

  // Adding "PlanarFourPointAngle"
  mitk::NodePredicateDataType::Pointer isPlanarFourPointAngle = mitk::NodePredicateDataType::New("PlanarFourPointAngle");
  descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarFourPointAngle"), QString(":/QtWidgetsExt/PlanarFourPointAngle_48.png"), isPlanarFourPointAngle, descriptorManager));

  // Adding "PlanarRectangle"
  mitk::NodePredicateDataType::Pointer isPlanarRectangle = mitk::NodePredicateDataType::New("PlanarRectangle");
  descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarRectangle"), QString(":/QtWidgetsExt/PlanarRectangle_48.png"), isPlanarRectangle, descriptorManager));

  // Adding "PlanarPolygon"
  mitk::NodePredicateDataType::Pointer isPlanarPolygon = mitk::NodePredicateDataType::New("PlanarPolygon");
  descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarPolygon"), QString(":/QtWidgetsExt/PlanarPolygon_48.png"), isPlanarPolygon, descriptorManager));

  // Adding "PlanarPath"
  mitk::NodePredicateProperty::Pointer isNotClosedPolygon = mitk::NodePredicateProperty::New("ClosedPlanarPolygon", mitk::BoolProperty::New(false));
  mitk::NodePredicateAnd::Pointer isPlanarPath = mitk::NodePredicateAnd::New(isNotClosedPolygon, isPlanarPolygon);
  descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarPath"), QString(":/QtWidgetsExt/PlanarPath_48.png"), isPlanarPath, descriptorManager));

  // Adding "PlanarDoubleEllipse"
  mitk::NodePredicateDataType::Pointer isPlanarDoubleEllipse = mitk::NodePredicateDataType::New("PlanarDoubleEllipse");
  descriptorManager->AddDescriptor(new QmitkNodeDescriptor("PlanarDoubleEllipse", ":/QtWidgetsExt/PlanarDoubleEllipse_48.png", isPlanarDoubleEllipse, descriptorManager));

  // Adding "PlanarBezierCurve"
  mitk::NodePredicateDataType::Pointer isPlanarBezierCurve = mitk::NodePredicateDataType::New("PlanarBezierCurve");
  descriptorManager->AddDescriptor(new QmitkNodeDescriptor("PlanarBezierCurve", ":/QtWidgetsExt/PlanarBezierCurve_48.png", isPlanarBezierCurve, descriptorManager));

  // Adding "PlanarSubdivisionPolygon"
  mitk::NodePredicateDataType::Pointer isPlanarSubdivisionPolygon = mitk::NodePredicateDataType::New("PlanarSubdivisionPolygon");
  descriptorManager->AddDescriptor(new QmitkNodeDescriptor("PlanarSubdivisionPolygon", ":/QtWidgetsExt/PlanarSubdivisionPolygon_48.png", isPlanarSubdivisionPolygon, descriptorManager));
}

void
mitk::PlanarFigureActivator::stop(ctkPluginContext* /*context*/)
{
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mitk_planarfigure, mitk::PlanarFigureActivator)
#endif
