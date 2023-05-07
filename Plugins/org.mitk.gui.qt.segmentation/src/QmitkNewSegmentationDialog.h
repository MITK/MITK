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

#include <vector>
#include <utility>

#include <QColor>
#include <QDialog>
#include <QString>

namespace mitk
{
  class LabelSetImage;
  class Label;
}

namespace Ui
{
  class QmitkNewSegmentationDialog;
}

/** \brief Dialog for naming labels.
 */
class MITK_QT_SEGMENTATION QmitkNewSegmentationDialog : public QDialog
{
  Q_OBJECT

public:
  using SuggestionsType = std::vector<std::pair<QString, QColor>>;

  enum Mode
  {
    NewLabel,
    RenameLabel
  };

  /** \brief Static helper function to (re)name a label.
   *
   * In NewLabel mode it is assumed that the label has not yet been added, hence for example a look-up table update is not done.
   * In RenameLabel mode the segmentation (if provided) is updated.
   */
  static bool DoRenameLabel(mitk::Label* label, mitk::LabelSetImage* segmentation, QWidget* parent = nullptr, Mode mode = NewLabel);

  explicit QmitkNewSegmentationDialog(QWidget *parent = nullptr, mitk::LabelSetImage* labelSetImage = nullptr, Mode mode = NewLabel);
  ~QmitkNewSegmentationDialog() override;

  QString GetName() const;
  mitk::Color GetColor() const;

  void SetName(const QString& name);
  void SetColor(const mitk::Color& color);

private:
  void OnAccept();
  void OnFinished(int result);
  void OnSuggestionSelected();
  void OnColorButtonClicked();
  void OnTextChanged(const QString& text);

  void SetSuggestions(const SuggestionsType& suggestions, bool replaceStandardSuggestions = false);
  void UpdateColorButtonBackground();
  void UpdateNameList();

  Ui::QmitkNewSegmentationDialog* m_Ui;

  bool m_SuggestOnce;

  QString m_Name;
  QColor m_Color;

  SuggestionsType m_Suggestions;
};

#endif
