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

#ifndef QMITKSEMANTICRELATIONSVIEW_H
#define QMITKSEMANTICRELATIONSVIEW_H

// semantic relations plugin
#include "ui_QmitkSemanticRelationsControls.h"

// semantic relations UI module
#include <QmitkPatientTableWidget.h>
#include <QmitkLesionInfoWidget.h>
#include <QmitkPatientInfoWidget.h>

// blueberry
#include <berryISelectionListener.h>

// mitk qt
#include <QmitkAbstractView.h>

/**
* @brief QmitkSemanticRelationsView
*/
class QmitkSemanticRelationsView : public QmitkAbstractView
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

protected:

  virtual void SetFocus() override;

  virtual void CreateQtPartControl(QWidget* parent) override;

private Q_SLOTS:

  void OnCaseIDSelectionChanged(const QString&);
  void OnPatientTableSelectionChanged(const mitk::DataNode*);

private:

  virtual void NodeAdded(const mitk::DataNode* node) override;
  virtual void NodeRemoved(const mitk::DataNode* node) override;

  // the Qt parent of our GUI
  QWidget* m_Parent;
  Ui::QmitkSemanticRelationsControls m_Controls;

  std::shared_ptr<mitk::SemanticRelations> m_SemanticRelations;
  QmitkPatientTableWidget* m_PatientTableWidget;
  QmitkLesionInfoWidget* m_LesionInfoWidget;
  QmitkPatientInfoWidget* m_PatientInfoWidget;
};

#endif // QMITKSEMANTICRELATIONSVIEW_H
