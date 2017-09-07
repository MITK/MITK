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

#ifndef QMITKLESIONINFOWIDGET_H
#define QMITKLESIONINFOWIDGET_H

// semantic relations UI module
#include "MitkSemanticRelationsUIExports.h"
#include <ui_QmitkLesionInfoWidgetControls.h>

// semantic relations module
#include <mitkSemanticRelations.h>
#include <mitkISemanticRelationsObserver.h>

// qt
#include <QWidget>

class MITKSEMANTICRELATIONSUI_EXPORT QmitkLesionInfoWidget : public QWidget, public mitk::ISemanticRelationsObserver
{
  Q_OBJECT

public:

  QmitkLesionInfoWidget::QmitkLesionInfoWidget(std::shared_ptr<mitk::SemanticRelations> semanticRelations, QWidget* parent = nullptr);
  ~QmitkLesionInfoWidget();

  // override observer interface
  virtual void Update(const mitk::SemanticTypes::CaseID& caseID) override;
  void ClearLesionInfoWidget();

Q_SIGNALS:
  void SelectionChanged(const mitk::DataNode*);

private Q_SLOTS:

  void OnLesionListItemClicked(const QModelIndex&);
  void OnLesionListSelectionChanged(const QItemSelection&, const QItemSelection&);

private:

  Ui::QmitkLesionInfoWidgetControls m_Controls;

  std::shared_ptr<mitk::SemanticRelations> m_SemanticRelations;
};

#endif // QMITKLESIONINFOWIDGET_H
