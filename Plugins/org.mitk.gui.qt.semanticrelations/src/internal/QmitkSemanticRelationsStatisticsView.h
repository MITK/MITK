/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKSEMANTICRELATIONSSTATISTICSVIEW_H
#define QMITKSEMANTICRELATIONSSTATISTICSVIEW_H

// semantic relations plugin
#include "ui_QmitkSemanticRelationsStatisticsControls.h"

// semantic relations module
#include <mitkSemanticTypes.h>

// semantic relations ui module
#include "QmitkStatisticsTreeModel.h"

// mitk qt gui common plugin
#include <QmitkAbstractView.h>

/*
* @brief The QmitkSemanticRelationsStatisticsView is an MITK view to combine and show the statistics tree view of the 'SemanticRelationsUI'-module.
*        It observes the semantic relations storage and displays the currently available case IDs in a combo box.
*        A 'QmitkStatisticsTreeModel' is created and set as the model of a QTreeView.
*/
class QmitkSemanticRelationsStatisticsView : public QmitkAbstractView, public mitk::ISemanticRelationsObserver
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  ~QmitkSemanticRelationsStatisticsView() override;

  /*
  * @brief Update the view with the data from the semantic relations.
  *
  *       Overridden from 'ISemanticRelationsObserver'.
  *       In order for the Update-function to be called, this view has to be added as an observer of SemanticRelation
  *       (e.g. m_SemanticRelations->AddObserver(this);)
  *
  * @par caseID    The current case ID to identify the currently active patient / case.
  */
  void Update(const mitk::SemanticTypes::CaseID& caseID) override;

protected:

  void SetFocus() override;
  void CreateQtPartControl(QWidget* parent) override;

private Q_SLOTS:

  void OnCaseIDSelectionChanged(const QString&);
  void OnModelUpdated();

private:

  void SetUpConnections();
  void AddToComboBox(const mitk::SemanticTypes::CaseID& caseID);

  Ui::QmitkSemanticRelationsStatisticsControls m_Controls;

  QmitkStatisticsTreeModel* m_StatisticsTreeModel;
};

#endif // QMITKSEMANTICRELATIONSSTATISTICSVIEW_H
