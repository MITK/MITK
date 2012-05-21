/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryQtMessageDialogTweaklet.h"

#include <QMessageBox>
#include <map>

namespace berry
{

class QtDialog: public IDialog
{
private:
  QMessageBox msgBox;
  QHash<QAbstractButton*, std::size_t> mapButtonToIndex;

public:

  QtDialog(Shell::Pointer, const QString& dialogTitle,
      void* /*dialogTitleImage*/, const QString& dialogMessage,
      int dialogImageType, const QList<QString>& dialogButtonLabels,
      int defaultIndex)
  {
    msgBox.setWindowTitle(dialogTitle);
    msgBox.setText(dialogMessage);

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
      QPushButton* button = msgBox.addButton(dialogButtonLabels[i],
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

QtMessageDialogTweaklet::QtMessageDialogTweaklet()
{

}

QtMessageDialogTweaklet::QtMessageDialogTweaklet(const QtMessageDialogTweaklet& other)
{

}

bool QtMessageDialogTweaklet::OpenConfirm(Shell::Pointer,
    const QString& title, const QString& message)
{
  QMessageBox msgBox;
  msgBox.setWindowTitle(title);
  msgBox.setText(message);
  msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Cancel);
  msgBox.setEscapeButton(QMessageBox::Cancel);
  return msgBox.exec() == QMessageBox::Ok;
}

void QtMessageDialogTweaklet::OpenError(Shell::Pointer,
    const QString& title, const QString& message)
{
  QMessageBox msgBox;
  msgBox.setWindowTitle(title);
  msgBox.setText(message);
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.setDefaultButton(QMessageBox::Ok);
  msgBox.setEscapeButton(QMessageBox::Ok);
  msgBox.setIcon(QMessageBox::Critical);
  msgBox.exec();
}

void QtMessageDialogTweaklet::OpenInformation(Shell::Pointer,
    const QString& title, const QString& message)
{
  QMessageBox msgBox;
  msgBox.setWindowTitle(title);
  msgBox.setText(message);
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.setDefaultButton(QMessageBox::Ok);
  msgBox.setEscapeButton(QMessageBox::Ok);
  msgBox.setIcon(QMessageBox::Information);
  msgBox.exec();
}

bool QtMessageDialogTweaklet::OpenQuestion(Shell::Pointer,
    const QString& title, const QString& message)
{
  QMessageBox msgBox;
  msgBox.setWindowTitle(title);
  msgBox.setText(message);
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::No);
  msgBox.setEscapeButton(QMessageBox::No);
  msgBox.setIcon(QMessageBox::Question);
  return msgBox.exec() == QMessageBox::Yes;
}

void QtMessageDialogTweaklet::OpenWarning(Shell::Pointer,
    const QString& title, const QString& message)
{
  QMessageBox msgBox;
  msgBox.setWindowTitle(title);
  msgBox.setText(message);
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.setDefaultButton(QMessageBox::Ok);
  msgBox.setEscapeButton(QMessageBox::Ok);
  msgBox.setIcon(QMessageBox::Warning);
  msgBox.exec();
}

IDialog::Pointer QtMessageDialogTweaklet::MessageDialog(
    Shell::Pointer parentShell, const QString& dialogTitle,
    void* dialogTitleImage, const QString& dialogMessage,
    int dialogImageType, const QList<QString>& dialogButtonLabels,
    int defaultIndex)
{
  IDialog::Pointer dialog(new QtDialog(parentShell, dialogTitle,
      dialogTitleImage, dialogMessage, dialogImageType, dialogButtonLabels,
      defaultIndex));
  return dialog;
}

}
