/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYQMITKPREFERENCESDIALOG_H_
#define BERRYQMITKPREFERENCESDIALOG_H_

#include <org_mitk_gui_qt_application_Export.h>

#include <QDialog>
#include <QScopedPointer>


class QmitkPreferencesDialogPrivate;

/**
 * \ingroup org_mitk_gui_qt_application
 */
class MITK_QT_APP QmitkPreferencesDialog : public QDialog
{
    Q_OBJECT

public:

    QmitkPreferencesDialog(QWidget * parent = nullptr, Qt::WindowFlags f = nullptr);
    ~QmitkPreferencesDialog() override;

    void SetSelectedPage(const QString& id);

protected slots:

  void OnImportButtonClicked();
  void OnExportButtonClicked();
  void OnDialogAccepted();
  void OnDialogRejected();

  void OnKeywordTextChanged(const QString & s);
  void OnKeywordEditingFinished();
  void OnPreferencesTreeItemSelectionChanged();

protected:

  //bool eventFilter(QObject *obj, QEvent *event);
  void UpdateTree();

  ///
  /// Saves all preferencepages.
  ///
  void SavePreferences();

  QScopedPointer<QmitkPreferencesDialogPrivate> d;

};

#endif /* BERRYQMITKPREFERENCESDIALOG_H_ */
