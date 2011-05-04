/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-09-12 15:46:48 +0200 (Fr, 12 Sep 2008) $
Version:   $Revision: 15236 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef QMITK_TOOLROIDATASELECTIONBOX_H
#define QMITK_TOOLROIDATASELECTIONBOX_H

#include "QmitkExtExports.h"
#include "mitkToolManager.h"
#include "QmitkBoundingObjectWidget.h"
#include <QComboBox>

/**
\brief Widget for defining a ROI inside the Interactive Segmentation Framwork

\ingroup ToolManagerEtAl
\ingroup Widgets

Allows to define a Region of interest (ROI) either by existing segmentations or by bounding objects.
Selection is possible via a combobox, listing all available segmentations. 
Item "bounding objects" activates the \ref QmitkBoundingObjectWidget.

*/
class QmitkExt_EXPORT QmitkToolRoiDataSelectionBox : public QWidget
{
  Q_OBJECT

public:

  QmitkToolRoiDataSelectionBox(QWidget* parent = 0, mitk::DataStorage* storage = 0);
  virtual ~QmitkToolRoiDataSelectionBox();

  mitk::DataStorage* GetDataStorage();
  void SetDataStorage(mitk::DataStorage& storage);

  mitk::ToolManager* GetToolManager();
  void SetToolManager(mitk::ToolManager& manager);

  void OnToolManagerRoiDataModified();

  void DataStorageChanged(const mitk::DataNode* node );

  mitk::ToolManager::DataVectorType GetSelection();

  void UpdateComboBoxData();

  void setEnabled(bool);

signals:
  
  void RoiDataSelected(const mitk::DataNode* node);

protected slots:

    void OnRoiDataSelectionChanged(const QString& name);
    void OnRoiDataSelectionChanged();

protected:

  QmitkBoundingObjectWidget* m_boundingObjectWidget;
  QComboBox* m_segmentationComboBox;

  mitk::ToolManager::Pointer m_ToolManager;
  bool m_SelfCall;

  mitk::DataNode::Pointer m_lastSelection;
  QString m_lastSelectedName;
};
#endif