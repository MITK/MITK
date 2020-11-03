/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKLEVELWINDOWPRESETDEFINITIONDIALOG_H_
#define QMITKLEVELWINDOWPRESETDEFINITIONDIALOG_H_

#include <MitkQtWidgetsExports.h>

#include "ui_QmitkLevelWindowPresetDefinition.h"

#include <QDialog>
#include <QSortFilterProxyModel>

#include <map>
#include <string>

/// \ingroup QmitkModule
class MITKQTWIDGETS_EXPORT QmitkLevelWindowPresetDefinitionDialog : public QDialog,
                                                                    public Ui::QmitkLevelWindowPresetDefinition
{
  Q_OBJECT

public:
  QmitkLevelWindowPresetDefinitionDialog(QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);
  ~QmitkLevelWindowPresetDefinitionDialog() override;

  void setPresets(std::map<std::string, double> &level,
                  std::map<std::string, double> &window,
                  QString initLevel,
                  QString initWindow);

  std::map<std::string, double> getLevelPresets();

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

  PresetTableModel *m_TableModel;
  QSortFilterProxyModel m_SortModel;
};

#endif /*QMITKLEVELWINDOWPRESETDEFINITIONDIALOG_H_*/
