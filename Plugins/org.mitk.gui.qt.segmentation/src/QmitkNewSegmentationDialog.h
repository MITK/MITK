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
#include <mitkLabelSetImage.h>

#include <vector>
#include <utility>

#include <QColor>
#include <QDialog>
#include <QString>

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
  enum class Mode
  {
    NewLabel,
    RenameLabel
  };

  /** \brief Static helper function to (re)name a label.
   *
   * In NewLabel mode it is assumed that the label has not yet been added, hence for example a look-up table update is not done.
   * In RenameLabel mode the segmentation (if provided) is updated.
   */
  static bool DoRenameLabel(mitk::Label* label, mitk::MultiLabelSegmentation* segmentation, QWidget* parent = nullptr, Mode mode = Mode::NewLabel);

  explicit QmitkNewSegmentationDialog(const mitk::MultiLabelSegmentation* labelSetImage,
    const mitk::Label* label = nullptr, Mode mode = Mode::NewLabel, QWidget* parent = nullptr);
  ~QmitkNewSegmentationDialog() override;

  QString GetName() const;
  mitk::Color GetColor() const;
  const mitk::Label* GetSuggestion() const;

private slots:
  void OnAccept();
  void OnFinished(int result);
  void OnSuggestionSelected();
  void OnSuggestionDoubleClicked();
  void OnColorButtonClicked();
  void OnNameEdited(const QString& text);
  void OnFilterEdited(const QString& text);
  void OnFilterClearClicked();
  void OnAutoFilterToggled(bool checked);

private:
  void InitializeDialog();
  void SetupConnections();
  void LoadAndApplyPreferences();
  void UpdateUI();
  void UpdateColorButtonBackground();
  void UpdateNameList();
  void UpdateSuggestionInfo();
  void UpdateOKButton();
  void UpdateControlStates();
  void ApplyFilter(const QString& filterText);
  void SelectSuggestionByName(const QString& name);
  bool TryPreselectMatchingSuggestion(const QString& name);
  bool GetAutoFilter() const;

  Ui::QmitkNewSegmentationDialog* m_Ui;

  Mode m_Mode;
  bool m_EnforceSuggestions;

  mitk::MultiLabelSegmentation::ConstLabelVectorType m_Suggestions;
  mitk::Label::Pointer m_Suggestion;
  QColor m_Color;
  QString m_Name;

  int m_TotalSuggestions;
  int m_VisibleSuggestions;
};

#endif
