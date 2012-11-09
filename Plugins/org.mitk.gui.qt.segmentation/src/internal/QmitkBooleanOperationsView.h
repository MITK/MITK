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

#ifndef QMITKBOOLEANOPERATIONSVIEW_H
#define QMITKBOOLEANOPERATIONSVIEW_H

#include <QmitkFunctionality.h>
#include <ui_QmitkBooleanOperationsView.h>

class QmitkBooleanOperationsView : public QmitkFunctionality
{
  Q_OBJECT

public:
  QmitkBooleanOperationsView();
  ~QmitkBooleanOperationsView();

protected:
  void CreateQtPartControl(QWidget *parent);

private slots:
  void OnSegmentationImage1Changed(const mitk::DataNode *dataNode);
  void OnSegmentationImage2Changed(const mitk::DataNode *dataNode);
  void OnDifferenceButtonClicked();
  void OnUnionButtonClicked();
  void OnIntersectionButtonClicked();

private:
  bool CheckSegmentationImages();
  mitk::Image::Pointer To3D(const mitk::Image::Pointer &image);
  void AddToDataStorage(const mitk::Image::Pointer &image, const std::string &prefix) const;
  void EnableButtons(bool enable = true);
  void DisableButtons();

  QWidget *m_Parent;
  Ui::QmitkBooleanOperationsView m_Controls;
};

#endif
