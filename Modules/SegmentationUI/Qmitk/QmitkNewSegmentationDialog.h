/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkNewSegmentationDialog_h_Included
#define QmitkNewSegmentationDialog_h_Included

#include <mitkColorProperty.h>
#include <mitkCommon.h>

#include <MitkSegmentationUIExports.h>

#include <QDialog>

namespace Ui
{
  class QmitkNewSegmentationDialog;
}

/**
  \brief Dialog for naming labels.
*/
class MITKSEGMENTATIONUI_EXPORT QmitkNewSegmentationDialog : public QDialog
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
  void SetSuggestionList(const QStringList& suggestionList);

private:
  void OnAccept();
  void OnSuggestionSelected(const QString& name);
  void OnColorButtonClicked();

  void UpdateColorButtonBackground();

  Ui::QmitkNewSegmentationDialog* m_Ui;

  QString m_Name;
  QColor m_Color;

  QStringList m_NameSuggestions;
  QList<QColor> m_ColorSuggestions;
};

#endif
