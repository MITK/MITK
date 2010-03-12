/*=========================================================================
 
 Program:   BlueBerry Platform
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

#include "berryQtMessageDialogTweaklet.h"

#include <QMessageBox>
#include <map>

namespace berry
{

class QtDialog: public IDialog
{
private:
  QMessageBox msgBox;
  std::map<QAbstractButton*, std::size_t> mapButtonToIndex;

public:

  QtDialog(Shell::Pointer parentShell, const std::string& dialogTitle,
      void* dialogTitleImage, const std::string& dialogMessage,
      int dialogImageType, const std::vector<std::string>& dialogButtonLabels,
      int defaultIndex)
  {
    msgBox.setWindowTitle(QString::fromStdString(dialogTitle));
    msgBox.setText(QString::fromStdString(dialogMessage));

    if (dialogImageType == IDialog::ERR)
      msgBox.setIcon(QMessageBox::Critical);
    else if (dialogImageType == IDialog::INFORMATION)
      msgBox.setIcon(QMessageBox::Information);
    else if (dialogImageType == IDialog::QUESTION)
      msgBox.setIcon(QMessageBox::Question);
    else if (dialogImageType == IDialog::WARNING)
      msgBox.setIcon(QMessageBox::Warning);


    QPushButton* defaultButton(NULL);
    for (std::size_t i = 0; i < dialogButtonLabels.size(); ++i)
    {
      QPushButton* button = msgBox.addButton(QString::fromStdString(dialogButtonLabels[i]),
          QMessageBox::ActionRole);
      mapButtonToIndex[(QAbstractButton*)button] = i;
      if (i == (std::size_t)defaultIndex)
        defaultButton = button;
    }

    msgBox.setDefaultButton(defaultButton);
  }

  int Open()
  {
    msgBox.exec();
    return static_cast<int>(mapButtonToIndex[msgBox.clickedButton()]);
  }
};

bool QtMessageDialogTweaklet::OpenConfirm(Shell::Pointer parent,
    const std::string& title, const std::string& message)
{
  QMessageBox msgBox;
  msgBox.setWindowTitle(QString::fromStdString(title));
  msgBox.setText(QString::fromStdString(message));
  msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Cancel);
  msgBox.setEscapeButton(QMessageBox::Cancel);
  return msgBox.exec() == QMessageBox::Ok;
}

void QtMessageDialogTweaklet::OpenError(Shell::Pointer parent,
    const std::string& title, const std::string& message)
{
  QMessageBox msgBox;
  msgBox.setWindowTitle(QString::fromStdString(title));
  msgBox.setText(QString::fromStdString(message));
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.setDefaultButton(QMessageBox::Ok);
  msgBox.setEscapeButton(QMessageBox::Ok);
  msgBox.setIcon(QMessageBox::Critical);
  msgBox.exec();
}

void QtMessageDialogTweaklet::OpenInformation(Shell::Pointer parent,
    const std::string& title, const std::string& message)
{
  QMessageBox msgBox;
  msgBox.setWindowTitle(QString::fromStdString(title));
  msgBox.setText(QString::fromStdString(message));
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.setDefaultButton(QMessageBox::Ok);
  msgBox.setEscapeButton(QMessageBox::Ok);
  msgBox.setIcon(QMessageBox::Information);
  msgBox.exec();
}

bool QtMessageDialogTweaklet::OpenQuestion(Shell::Pointer parent,
    const std::string& title, const std::string& message)
{
  QMessageBox msgBox;
  msgBox.setWindowTitle(QString::fromStdString(title));
  msgBox.setText(QString::fromStdString(message));
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::No);
  msgBox.setEscapeButton(QMessageBox::No);
  msgBox.setIcon(QMessageBox::Question);
  return msgBox.exec() == QMessageBox::Yes;
}

void QtMessageDialogTweaklet::OpenWarning(Shell::Pointer parent,
    const std::string& title, const std::string& message)
{
  QMessageBox msgBox;
  msgBox.setWindowTitle(QString::fromStdString(title));
  msgBox.setText(QString::fromStdString(message));
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.setDefaultButton(QMessageBox::Ok);
  msgBox.setEscapeButton(QMessageBox::Ok);
  msgBox.setIcon(QMessageBox::Warning);
  msgBox.exec();
}

IDialog::Pointer QtMessageDialogTweaklet::MessageDialog(
    Shell::Pointer parentShell, const std::string& dialogTitle,
    void* dialogTitleImage, const std::string& dialogMessage,
    int dialogImageType, const std::vector<std::string>& dialogButtonLabels,
    int defaultIndex)
{
  IDialog::Pointer dialog(new QtDialog(parentShell, dialogTitle,
      dialogTitleImage, dialogMessage, dialogImageType, dialogButtonLabels,
      defaultIndex));
  return dialog;
}

}
