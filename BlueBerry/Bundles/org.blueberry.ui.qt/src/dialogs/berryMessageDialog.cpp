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

#include "tweaklets/berryMessageDialogTweaklet.h"

#include "berryMessageDialog.h"

namespace berry
{

bool MessageDialog::OpenConfirm(Shell::Pointer parent,
    const QString& title, const QString& message)
{
  return Tweaklets::Get(MessageDialogTweaklet::KEY)->OpenConfirm(parent, title,
      message);
}

void MessageDialog::OpenError(Shell::Pointer parent, const QString& title,
    const QString& message)
{
  Tweaklets::Get(MessageDialogTweaklet::KEY)->OpenError(parent, title, message);
}

void MessageDialog::OpenInformation(Shell::Pointer parent,
    const QString& title, const QString& message)
{
  Tweaklets::Get(MessageDialogTweaklet::KEY)->OpenInformation(parent, title,
      message);
}

bool MessageDialog::OpenQuestion(Shell::Pointer parent,
    const QString& title, const QString& message)
{
  return Tweaklets::Get(MessageDialogTweaklet::KEY)->OpenQuestion(parent, title,
      message);
}

void MessageDialog::OpenWarning(Shell::Pointer parent,
    const QString& title, const QString& message)
{
  Tweaklets::Get(MessageDialogTweaklet::KEY)->OpenWarning(parent, title,
      message);
}

IDialog::Pointer MessageDialog::CreateMessageDialog(Shell::Pointer parentShell, const QString& dialogTitle,
         void* dialogTitleImage, const QString& dialogMessage, int dialogImageType,
         const QList<QString>& dialogButtonLabels, int defaultIndex)
{
  return Tweaklets::Get(MessageDialogTweaklet::KEY)->MessageDialog(parentShell, dialogTitle,
      dialogTitleImage, dialogMessage, dialogImageType,
      dialogButtonLabels, defaultIndex);
}

}
