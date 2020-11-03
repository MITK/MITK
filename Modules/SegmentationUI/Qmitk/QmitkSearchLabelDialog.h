/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSearchLabelDialog_h_Included
#define QmitkSearchLabelDialog_h_Included

#include "MitkSegmentationUIExports.h"

#include <ui_QmitkSearchLabelDialogGUI.h>

#include <QCompleter>
#include <QDialog>

class MITKSEGMENTATIONUI_EXPORT QmitkSearchLabelDialog : public QDialog
{
  Q_OBJECT

public:
  QmitkSearchLabelDialog(QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);
  ~QmitkSearchLabelDialog() override;

  int GetLabelSetWidgetTableIndex();

  QString GetLabelSetWidgetTableCompleteWord();

  void SetLabelSuggestionList(QStringList stringList);

signals:

  void goToLabel(int);

public slots:

protected slots:

  void OnLabelCompleterChanged(const QString &completedWord);

protected:
  Ui::QmitkSearchLabelDialogGUI *m_Controls;

  QCompleter *m_Completer;

  QStringList m_LabelList;

  int m_LabelIndex;

  QString m_CompleteWord;
};

#endif
