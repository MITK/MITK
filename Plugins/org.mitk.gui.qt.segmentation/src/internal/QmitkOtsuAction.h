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
#ifndef QMITKOTSUACTION_H
#define QMITKOTSUACTION_H

#include <org_mitk_gui_qt_segmentation_Export.h>

// Parent classes
#include <QObject>
#include <mitkIContextMenuAction.h>

// Data members
#include <mitkDataStorage.h>
#include <QSpinBox>
#include <QPushButton>

// Mitk classes
#include <mitkDataNode.h>

class QDialog;
class QmitkStdMultiWidget;

class MITK_QT_SEGMENTATION QmitkOtsuAction : public QObject, public mitk::IContextMenuAction
{
  Q_OBJECT
  Q_INTERFACES(mitk::IContextMenuAction)

public:
  QmitkOtsuAction();
  ~QmitkOtsuAction();

  // IContextMenuAction
  void Run(const QList<mitk::DataNode::Pointer> &selectedNodes);
  void SetDataStorage(mitk::DataStorage *dataStorage);
  void SetFunctionality(berry::QtViewPart *functionality);
  void SetSmoothed(bool smoothed){}
  void SetDecimated(bool decimated){}

private slots:
  void OtsuSegmentationDone();

private:
  QmitkOtsuAction(const QmitkOtsuAction &);
  QmitkOtsuAction & operator=(const QmitkOtsuAction &);

  void PerformOtsuSegmentation();

  mitk::DataStorage::Pointer m_DataStorage;
  QDialog *m_OtsuSegmentationDialog;

  QSpinBox* m_OtsuSpinBox;
  QPushButton* m_OtsuPushButton;

  mitk::DataNode::Pointer m_DataNode;
};

#endif
