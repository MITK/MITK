/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKSEMANTICRELATIONSNODESELECTIONDIALOG_H
#define QMITKSEMANTICRELATIONSNODESELECTIONDIALOG_H

// semantic relations module
#include <mitkSemanticTypes.h>

// org.mitk.gui.qt.common plugin
#include <QmitkNodeSelectionDialog.h>

/*
* @brief The QmitkSemanticRelationsNodeSelectionDialog extends the QmitkNodeSelectionDialog so that a case ID and a lesion
*        can be set for each known panel that is of type 'QmitkSemanticRelationsStorageInspector'.
*/
class QmitkSemanticRelationsNodeSelectionDialog : public QmitkNodeSelectionDialog
{
  Q_OBJECT

public:

  explicit QmitkSemanticRelationsNodeSelectionDialog(QWidget* parent = nullptr, QString caption = "", QString hint = "");
  /**
  * @brief Extends the base class to allow setting the current case ID which is needed to access the
  *        semantic relations storage. The function sets the case ID of each 'QmitkSemanticRelationsStorageInspector'.
  *
  * @param caseID    A case ID as string
  */
  virtual void SetCaseID(const mitk::SemanticTypes::CaseID& caseID);
  /**
  * @brief Extends the base class to allow setting the current lesion which can be used to show on which images
  *        the lesion is visible. The function sets the lesion of each 'QmitkSemanticRelationsStorageInspector'.
  *
  * @param lesion   The selected lesion
  */
  virtual void SetLesion(const mitk::SemanticTypes::Lesion& lesion);

};

#endif // QMITKSEMANTICRELATIONSNODESELECTIONDIALOG_H
