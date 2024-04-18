/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QDialog>

class QItemSelection;

namespace Ui
{
  class QmitkStartupDialog;
}

class QmitkStartupDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QmitkStartupDialog(QWidget* parent = nullptr);
  ~QmitkStartupDialog() override;

  bool UsePreset() const;
  QString GetPresetName() const;
  QStringList GetPresetCategories() const;

private:
  bool SkipDialog() const;
  void LoadPresets();

  void showEvent(QShowEvent* event) override;

  void OnPresetRadioButtonToggled(bool checked);
  void OnSelectedPresetChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void OnFinished(int result);

  Ui::QmitkStartupDialog* m_Ui;
};
