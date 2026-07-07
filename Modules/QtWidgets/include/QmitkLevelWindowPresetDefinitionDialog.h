/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkLevelWindowPresetDefinitionDialog_h
#define QmitkLevelWindowPresetDefinitionDialog_h

#include <MitkQtWidgetsExports.h>

#include <QDialog>
#include <QSortFilterProxyModel>

#include <map>
#include <string>
#include <memory>

namespace Ui
{
  class QmitkLevelWindowPresetDefinition;
}

/** \brief Dialog for defining and managing level/window presets.
 *
 * Provides a table-based UI for adding, removing, and editing named
 * level/window presets. Presets can be sorted by name, level, or window value.
 *
 * \ingroup QmitkModule
 * \sa QmitkLevelWindowWidgetContextMenu, QmitkLevelWindowRangeChangeDialog
 */
class MITKQTWIDGETS_EXPORT QmitkLevelWindowPresetDefinitionDialog : public QDialog
{
  Q_OBJECT

public:
  /** \brief Construct the preset definition dialog.
   * \param[in] parent Parent widget.
   * \param[in] f Window flags.
   */
  QmitkLevelWindowPresetDefinitionDialog(QWidget *parent = nullptr, Qt::WindowFlags f = {});
  ~QmitkLevelWindowPresetDefinitionDialog() override;

  /** \brief Initialize the dialog with existing presets and default values.
   * \param[in] level  Map of preset names to level values.
   * \param[in] window Map of preset names to window values.
   * \param[in] initLevel  Default level value shown in the input field.
   * \param[in] initWindow Default window value shown in the input field.
   */
  void setPresets(std::map<std::string, double> &level,
                  std::map<std::string, double> &window,
                  QString initLevel,
                  QString initWindow);

  /** \brief Get the edited level presets.
   * \return Map of preset names to level values.
   */
  std::map<std::string, double> getLevelPresets();

  /** \brief Get the edited window presets.
   * \return Map of preset names to window values.
   */
  std::map<std::string, double> getWindowPresets();

protected slots:

  void addPreset();
  void removePreset();
  void changePreset();

  void ListViewSelectionChanged(const QItemSelection &, const QItemSelection &);
  void sortPresets(int index);

protected:
  class PresetTableModel : public QAbstractTableModel
  {
  public:
    struct Entry
    {
      std::string name;
      double level;
      double window;

      Entry(const std::string &n, double l, double w) : name(n), level(l), window(w) {}
    };

    PresetTableModel(std::map<std::string, double> &levels,
                     std::map<std::string, double> &windows,
                     QObject *parent = nullptr);

    int rowCount(const QModelIndex &) const override;
    int columnCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &index, int) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int) const override;

    void addPreset(std::string &name, double level, double window);
    void removePreset(const QModelIndex &);
    void changePreset(int row, std::string &name, double level, double window);

    void getLevels(std::map<std::string, double> &levels);
    void getWindows(std::map<std::string, double> &windows);

    bool contains(std::string &name);

    Entry getPreset(const QModelIndex &) const;

  private:
    std::vector<Entry> m_Entries;
  };

  void resizeEvent(QResizeEvent *event) override;
  void showEvent(QShowEvent *event) override;

  void resizeColumns();

  std::unique_ptr<Ui::QmitkLevelWindowPresetDefinition> m_Controls;
  PresetTableModel *m_TableModel;
  QSortFilterProxyModel m_SortModel;
};

#endif
