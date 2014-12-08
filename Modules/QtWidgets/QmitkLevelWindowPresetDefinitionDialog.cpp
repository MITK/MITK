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

#include "QmitkLevelWindowPresetDefinitionDialog.h"

#include <QMessageBox>
#include <QTableWidgetItem>
#include <QHeaderView>

QmitkLevelWindowPresetDefinitionDialog::QmitkLevelWindowPresetDefinitionDialog(QWidget* parent, Qt::WindowFlags f)
 : QDialog(parent, f), m_TableModel(0), m_SortModel(this)
{
  this->setupUi(this);

  QObject::connect(addButton, SIGNAL(clicked()), this, SLOT(addPreset()));
  QObject::connect(removeButton, SIGNAL(clicked()), this, SLOT(removePreset()));
  QObject::connect(changeButton, SIGNAL(clicked()), this, SLOT(changePreset()));

  QObject::connect(presetView->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sortPresets(int)));

  presetView->verticalHeader()->setVisible(false);
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  presetView->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
#else
  presetView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
#endif

  presetView->setModel(&m_SortModel);
}

QmitkLevelWindowPresetDefinitionDialog::~QmitkLevelWindowPresetDefinitionDialog()
{
  delete m_TableModel;
}

void QmitkLevelWindowPresetDefinitionDialog::sortPresets(int index)
{
  static Qt::SortOrder order[3] = {Qt::AscendingOrder};

  presetView->sortByColumn(index, order[index]);
  if (order[index] == Qt::AscendingOrder) order[index] = Qt::DescendingOrder;
  else order[index] = Qt::AscendingOrder;
}

void QmitkLevelWindowPresetDefinitionDialog::resizeEvent(QResizeEvent* event)
{
  QDialog::resizeEvent(event);

  this->resizeColumns();
}

void QmitkLevelWindowPresetDefinitionDialog::showEvent(QShowEvent* event)
{
  this->resizeColumns();

  QDialog::showEvent(event);
}

void QmitkLevelWindowPresetDefinitionDialog::resizeColumns()
{
  int width = presetView->viewport()->size().width() - presetView->columnWidth(1) - presetView->columnWidth(2);
  if (width < 50) width = 50;

  presetView->setColumnWidth(0, width);
}

void QmitkLevelWindowPresetDefinitionDialog::addPreset()
{
  std::string name(presetnameLineEdit->text().toStdString());
  if (m_TableModel->contains(name))
  {
    QMessageBox::critical( this, "Preset definition",
    "Presetname already exists.\n"
    "You have to enter another one." );
  }
  else if (presetnameLineEdit->text() == "")
  {
    QMessageBox::critical( this, "Preset definition",
    "Presetname has to be set.\n"
    "You have to enter a Presetname." );
  }
  else
  {
    m_TableModel->addPreset(name, levelSpinBox->value(), windowSpinBox->value());
  }
}


void QmitkLevelWindowPresetDefinitionDialog::removePreset()
{
  QModelIndex index(presetView->selectionModel()->currentIndex());
  if (!index.isValid()) return;

  m_TableModel->removePreset(index);
}


void QmitkLevelWindowPresetDefinitionDialog::changePreset()
{
  if (presetView->selectionModel()->hasSelection())
  {
    std::string name(presetnameLineEdit->text().toStdString());
    if (name == "")
    {
      QMessageBox::critical( this, "Preset definition",
      "Presetname has to be set.\n"
      "You have to enter a Presetname." );
    }
    else if (m_TableModel->contains(name)
        && (m_TableModel->getPreset(presetView->selectionModel()->currentIndex()).name
            != name))
    {
      QMessageBox::critical( this, "Preset definition",
      "Presetname already exists.\n"
      "You have to enter another one." );
    }
    else
    {
      m_TableModel->changePreset(presetView->selectionModel()->currentIndex().row(),
                                 name, levelSpinBox->value(), windowSpinBox->value());
    }
  }
}


void QmitkLevelWindowPresetDefinitionDialog::setPresets(std::map<std::string, double>& level, std::map<std::string, double>& window, QString initLevel, QString initWindow)
{
  levelSpinBox->setValue(initLevel.toInt());
  windowSpinBox->setValue(initWindow.toInt());

  delete m_TableModel;
  m_TableModel = new PresetTableModel(level, window, this);

  m_SortModel.setSourceModel(m_TableModel);

  QObject::connect(presetView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(ListViewSelectionChanged(const QItemSelection&, const QItemSelection&)));

  this->sortPresets(0);

  presetView->resizeColumnsToContents();
}


std::map<std::string, double> QmitkLevelWindowPresetDefinitionDialog::getLevelPresets()
{
  std::map<std::string, double> levels;
  m_TableModel->getLevels(levels);
  return levels;
}


std::map<std::string, double> QmitkLevelWindowPresetDefinitionDialog::getWindowPresets()
{
  std::map<std::string, double> windows;
  m_TableModel->getWindows(windows);
  return windows;
}


void QmitkLevelWindowPresetDefinitionDialog::ListViewSelectionChanged(const QItemSelection& selected,
    const QItemSelection& /*deselected*/)
{
  QModelIndexList indexes(selected.indexes());
  if (indexes.empty())
  {
    presetnameLineEdit->setText("");
    levelSpinBox->setValue(0);
    windowSpinBox->setValue(0);
  }
  else
  {
    //use the sorted index to get the entry
    PresetTableModel::Entry entry( m_TableModel->getPreset((m_SortModel.mapToSource(indexes.first()))) );
    presetnameLineEdit->setText(QString(entry.name.c_str()));
    levelSpinBox->setValue((int)entry.level);
    windowSpinBox->setValue((int)entry.window);
  }
}

QmitkLevelWindowPresetDefinitionDialog::
PresetTableModel::PresetTableModel(std::map<std::string, double>& levels,
    std::map<std::string, double>& windows, QObject* parent)
 : QAbstractTableModel(parent)
{
  for(std::map<std::string, double>::iterator iter = levels.begin(); iter != levels.end(); ++iter )
  {
    m_Entries.push_back(Entry(iter->first, iter->second, windows[iter->first]));
  }
}

void
QmitkLevelWindowPresetDefinitionDialog::
PresetTableModel::getLevels(std::map<std::string, double>& levels)
{
  for (std::vector<Entry>::iterator iter = m_Entries.begin(); iter != m_Entries.end(); ++iter)
  {
    levels[iter->name] = iter->level;
  }
}

void
QmitkLevelWindowPresetDefinitionDialog::
PresetTableModel::getWindows(std::map<std::string, double>& windows)
{
  for (std::vector<Entry>::iterator iter = m_Entries.begin(); iter != m_Entries.end(); ++iter)
  {
    windows[iter->name] = iter->window;
  }
}

void
QmitkLevelWindowPresetDefinitionDialog::
PresetTableModel::addPreset(std::string& name, double level, double window)
{

  this->beginInsertRows(QModelIndex(), (int) m_Entries.size(), (int) m_Entries.size());

  m_Entries.push_back(Entry(name, level, window));

  this->endInsertRows();
}

bool
QmitkLevelWindowPresetDefinitionDialog::
PresetTableModel::contains(std::string& name)
{

  for (std::vector<Entry>::iterator iter = m_Entries.begin(); iter != m_Entries.end(); ++iter)
  {
    if (iter->name == name) return true;
  }

  return false;
}

void
QmitkLevelWindowPresetDefinitionDialog::
PresetTableModel::removePreset(const QModelIndex& index)
{
  int row = index.row();

  this->beginRemoveRows(QModelIndex(), row, row);

  m_Entries.erase(m_Entries.begin()+row);

  this->endRemoveRows();
}

void
QmitkLevelWindowPresetDefinitionDialog::
PresetTableModel::changePreset(int row, std::string& name, double level, double window)
{
  m_Entries[row].name = name;
  m_Entries[row].level = level;
  m_Entries[row].window = window;

  this->dataChanged(index(row, 0), index(row, 2));
}


QmitkLevelWindowPresetDefinitionDialog::PresetTableModel::Entry
QmitkLevelWindowPresetDefinitionDialog::
PresetTableModel::getPreset(const QModelIndex& index) const
{
  int row = index.row();

  if (row < 0 || (unsigned int)row >= m_Entries.size()) return Entry("", 0, 0);

  return m_Entries[row];
}

int
QmitkLevelWindowPresetDefinitionDialog::
PresetTableModel::rowCount(const QModelIndex&) const
{
  return (int) m_Entries.size();
}

int
QmitkLevelWindowPresetDefinitionDialog::
PresetTableModel::columnCount(const QModelIndex&) const
{
  return 3;
}

QVariant
QmitkLevelWindowPresetDefinitionDialog::
PresetTableModel::data(const QModelIndex& index, int role) const
{
  if (role == Qt::DisplayRole)
  {
    switch (index.column()) {
    case 0:
      return QVariant(QString(m_Entries[index.row()].name.c_str()));
    case 1:
    {
      return QVariant(m_Entries[index.row()].level);
    }
    case 2:
      return QVariant(m_Entries[index.row()].window);
    }
  }

  return QVariant();
}

QVariant
QmitkLevelWindowPresetDefinitionDialog::
PresetTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
  {
    switch (section) {
    case 0: return QVariant("Preset");
    case 1: return QVariant("Level");
    case 2: return QVariant("Window");
    }
  }

  return QVariant();
}
