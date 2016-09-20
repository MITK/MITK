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

#ifndef QmitkSearchLabelDialog_h_Included
#define QmitkSearchLabelDialog_h_Included

#include "MitkSegmentationUIExports.h"

#include <ui_QmitkSearchLabelDialogGUI.h>

#include <QDialog>
#include <QCompleter>

class MITKSEGMENTATIONUI_EXPORT QmitkSearchLabelDialog : public QDialog
{
  Q_OBJECT

  public:

  QmitkSearchLabelDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
     virtual ~QmitkSearchLabelDialog();

    int GetLabelSetWidgetTableIndex();

    QString GetLabelSetWidgetTableCompleteWord();

    void SetLabelSuggestionList(QStringList stringList);

signals:

   void goToLabel(int);

  public slots:

  protected slots:

    void OnLabelCompleterChanged(const QString& completedWord);

  protected:

    Ui::QmitkSearchLabelDialogGUI *m_Controls;

    QCompleter* m_Completer;

    QStringList m_LabelList;

    int m_LabelIndex;

    QString m_CompleteWord;

};

#endif
