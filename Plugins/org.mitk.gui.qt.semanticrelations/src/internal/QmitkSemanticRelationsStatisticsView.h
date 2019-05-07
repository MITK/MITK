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

  virtual ~QmitkSemanticRelationsStatisticsView() override;

  /*
  * @brief Update the view with the data from the semantic relations.
  *
  *       Overridden from 'ISemanticRelationsObserver'.
  *       In order for the Update-function to be called, this view has to be added as an observer of SemanticRelation
  *       (e.g. m_SemanticRelations->AddObserver(this);)
  *
  * @par caseID    The current case ID to identify the currently active patient / case.
  */
  virtual void Update(const mitk::SemanticTypes::CaseID& caseID) override;

protected:

  virtual void SetFocus() override;
  virtual void CreateQtPartControl(QWidget* parent) override;

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
