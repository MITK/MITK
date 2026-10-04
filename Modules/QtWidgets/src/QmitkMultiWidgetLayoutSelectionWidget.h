/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMultiWidgetLayoutSelectionWidget_h
#define QmitkMultiWidgetLayoutSelectionWidget_h

#include <QmitkAutomatedLayoutWidget.h>

#include <nlohmann/json.hpp>

// qt
#include <QStringList>
#include <QWidget>

#include <iosfwd>
#include <memory>
#include <vector>

namespace Ui
{
  class QmitkMultiWidgetLayoutSelectionWidget;
}

/**
* \brief The layout-shape picker of an MxN editor: a table to define a custom
*        row/column arrangement, the data-based alternative, and the file and
*        preset actions of the layout document.
*
* Module-internal. The surface a hosting view sees is QmitkMxNLayoutEditorWidget,
* which shows the picker on demand in a dialog, drives the file and preset
* actions from its own header row, and forwards every signal below. The widget
* reports what the user asked for and never touches an editor itself, so the
* host stays free to gate the destructive paths.
*/
class QmitkMultiWidgetLayoutSelectionWidget : public QWidget
{
  Q_OBJECT

public:

  QmitkMultiWidgetLayoutSelectionWidget(QWidget* parent = nullptr);
  ~QmitkMultiWidgetLayoutSelectionWidget() override;

  void SetDataStorage(mitk::DataStorage* dataStorage);

  /**
   * \brief Clear the transient grid-table selection, so the picker opens fresh
   *        rather than showing the previous pick. Called by a host that re-shows
   *        the widget on demand (the layout editor's "Edit grid..." dialog)
   *        instead of keeping it always visible.
   */
  void ResetSelection();

  /**
   * \brief The display names of the layout presets shipped as module
   *        resources, indexed the way ApplyPreset expects.
   */
  QStringList PresetNames() const;

public Q_SLOTS:

  /** \brief Ask for a target file and emit SaveLayout writing into it. */
  void RequestSave();

  /** \brief Ask for a layout file and emit LoadLayout with its parsed content. */
  void RequestLoad();

  /** \brief Emit LoadLayout with the preset at 'index' into PresetNames. */
  void ApplyPreset(int index);

Q_SIGNALS:

  void LayoutSet(int row, int column);
  void SetDataBasedLayout(const QList<mitk::DataNode::Pointer>& nodes);

  /** \brief The data-based chooser is about to open as a popup of its own; a
   *         host that shows this picker in a dialog closes the dialog, so a
   *         dismissed chooser leaves nothing behind. The picker survives the
   *         host's reaction. */
  void DataBasedLayoutStarted();

  // needs to be connected via Qt::DirectConnection (usually default), to ensure the stream pointers validity
  void SaveLayout(std::ostream* outStream);

  void LoadLayout(const nlohmann::json* jsonData);

private Q_SLOTS:

  void OnTableItemSelectionChanged();
  void OnSetLayoutButtonClicked();
  void OnDataBasedLayoutButtonClicked();

private:

  void Init();

  struct Preset
  {
    QString name;
    nlohmann::json layout;
  };

  std::unique_ptr<Ui::QmitkMultiWidgetLayoutSelectionWidget> ui;
  std::vector<Preset> m_Presets;
  QmitkAutomatedLayoutWidget* m_AutomatedDataLayoutWidget;

};

#endif
