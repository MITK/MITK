/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkNewSegmentationDialog_h
#define QmitkNewSegmentationDialog_h

#include <org_mitk_gui_qt_segmentation_Export.h>

#include <mitkColorProperty.h>

#include <map>

#include <QColor>
#include <QDialog>
#include <QString>

namespace Ui
{
  class QmitkNewSegmentationDialog;
}

/**
  \brief Dialog for naming labels.
*/
class MITK_QT_SEGMENTATION QmitkNewSegmentationDialog : public QDialog
{
  Q_OBJECT

public:
  enum Mode
  {
    NewLabel,
    RenameLabel
  };

  explicit QmitkNewSegmentationDialog(QWidget *parent = nullptr, Mode mode = NewLabel);
  ~QmitkNewSegmentationDialog() override;

  QString GetName() const;
  mitk::Color GetColor() const;

  void SetName(const QString& name);
  void SetColor(const mitk::Color& color);

private:
  void OnAccept();
  void OnSuggestionSelected(const QString& name);
  void OnColorButtonClicked();

  void SetSuggestions(const std::map<QString, QColor> suggestions, bool replaceStandardSuggestions = false);
  void UpdateColorButtonBackground();

  Ui::QmitkNewSegmentationDialog* m_Ui;

  bool m_SuggestOnce;

  QString m_Name;
  QColor m_Color;

  std::map<QString, QColor> m_Suggestions;
};

#endif
