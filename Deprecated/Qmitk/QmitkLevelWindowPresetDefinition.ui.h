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

/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/


void QmitkLevelWindowPresetDefinition::addPreset()
{
  if (presetList->findItem(presetnameLineEdit->text(), 0) && presetList->selectedItem() != presetList->findItem(presetnameLineEdit->text(), 0))
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
    presetList->insertItem(new QListViewItem( presetList, presetnameLineEdit->text(), QString::number(levelSpinBox->value()), QString::number(windowSpinBox->value())));
  }
}


void QmitkLevelWindowPresetDefinition::removePreset()
{
  presetList->takeItem(presetList->selectedItem());
}


void QmitkLevelWindowPresetDefinition::changePreset()
{
  if (presetList->selectedItem())
  {
    if (presetnameLineEdit->text() == "")
    {
      QMessageBox::critical( this, "Preset definition",
      "Presetname has to be set.\n"
      "You have to enter a Presetname." );
    }
    else if (presetList->findItem(presetnameLineEdit->text(), 0) && (presetList->selectedItem() != presetList->findItem(presetnameLineEdit->text(), 0)))
    {
      QMessageBox::critical( this, "Preset definition",
      "Presetname already exists.\n"
      "You have to enter another one." );
    }
    else
    {
      QListViewItem* changedItem = new QListViewItem(presetList, presetnameLineEdit->text(), QString::number(levelSpinBox->value()), QString::number(windowSpinBox->value()));
      presetList->takeItem(presetList->selectedItem());
      presetList->insertItem(changedItem);
    }
  }
}


void QmitkLevelWindowPresetDefinition::setPresets(std::map<std::string, double>& level, std::map<std::string, double>& window, QString initLevel, QString initWindow)
{
  levelSpinBox->setValue(initLevel.toInt());
  windowSpinBox->setValue(initWindow.toInt());
  for( std::map<std::string, double>::iterator iter = level.begin(); iter != level.end(); ++iter )
  {
    std::string item = iter->first;
    presetList->insertItem(new QListViewItem(presetList, QString(item.c_str()), QString::number(level[item]), QString::number(window[item])));
  }
}


std::map<std::string, double> QmitkLevelWindowPresetDefinition::getLevelPresets()
{
  QListViewItem* item = presetList->firstChild();
  while (item)
  {
    newLevel[std::string((const char*)item->text(0))] = item->text(1).toDouble();
    item = item->nextSibling();
  }
  return newLevel;
}


std::map<std::string, double> QmitkLevelWindowPresetDefinition::getWindowPresets()
{
  QListViewItem* item = presetList->firstChild();
  while (item)
  {
    newWindow[std::string((const char*)item->text(0))] = item->text(2).toDouble();
    item = item->nextSibling();
  }
  return newWindow;
}


void QmitkLevelWindowPresetDefinition::ListViewSelectionChanged( QListViewItem * preset)
{
  presetnameLineEdit->setText(preset->text(0));
  levelSpinBox->setValue(preset->text(1).toInt());
  windowSpinBox->setValue(preset->text(2).toInt());
}
