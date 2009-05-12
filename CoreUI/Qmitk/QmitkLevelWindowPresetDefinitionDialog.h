/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QMITKLEVELWINDOWPRESETDEFINITIONDIALOG_H_
#define QMITKLEVELWINDOWPRESETDEFINITIONDIALOG_H_

#include "ui_QmitkLevelWindowPresetDefinition.h"

#include <QDialog>
#include <QSortFilterProxyModel>

#include <map>
#include <string>

#include <mitkCommon.h>

class QMITK_EXPORT QmitkLevelWindowPresetDefinitionDialog : public QDialog, public Ui::QmitkLevelWindowPresetDefinition
{
  Q_OBJECT
  
public:
  
  QmitkLevelWindowPresetDefinitionDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~QmitkLevelWindowPresetDefinitionDialog();
  
  void setPresets(std::map<std::string, double>& level, std::map<std::string, double>& window, QString initLevel, QString initWindow);

  std::map<std::string, double> getLevelPresets();

  std::map<std::string, double> getWindowPresets();

protected slots:

  void addPreset();
  void removePreset();
  void changePreset();
  
  void ListViewSelectionChanged(const QItemSelection&, const QItemSelection&);
  void sortPresets(int index);
 
  
protected:
  
  class PresetTableModel : public QAbstractTableModel
  {
      
  public:
    
    struct Entry {
      std::string name;
      double level;
      double window;
      
      Entry(const std::string& n, double l, double w)
       : name(n), level(l), window(w) {}
    };
    
    PresetTableModel(std::map<std::string, double>& levels,
                     std::map<std::string, double>& windows,
                     QObject* parent = 0);
    
    int rowCount(const QModelIndex&) const;
    int columnCount(const QModelIndex&) const;
    QVariant data(const QModelIndex& index, int) const;
    
    QVariant headerData(int section, Qt::Orientation orientation, int) const;
   
    void addPreset(std::string& name, double level, double window);
    void removePreset(const QModelIndex&);
    void changePreset(int row, std::string& name, double level, double window);
    
    void getLevels(std::map<std::string, double>& levels);
    void getWindows(std::map<std::string, double>& windows);
    
    bool contains(std::string& name);
  
    Entry getPreset(const QModelIndex&) const;
  
  private:
        
     std::vector<Entry> m_Entries;
   
  };
  
  void resizeEvent(QResizeEvent* event);
  void showEvent(QShowEvent* event);
  
  void resizeColumns();
  
  PresetTableModel* m_TableModel;
  QSortFilterProxyModel m_SortModel;
};

#endif /*QMITKLEVELWINDOWPRESETDEFINITIONDIALOG_H_*/
