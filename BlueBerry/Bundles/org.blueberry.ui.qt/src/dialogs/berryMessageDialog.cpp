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
    const std::string& title, const std::string& message)
{
  return Tweaklets::Get(MessageDialogTweaklet::KEY)->OpenConfirm(parent, title,
      message);
}

void MessageDialog::OpenError(Shell::Pointer parent, const std::string& title,
    const std::string& message)
{
  Tweaklets::Get(MessageDialogTweaklet::KEY)->OpenError(parent, title, message);
}

void MessageDialog::OpenInformation(Shell::Pointer parent,
    const std::string& title, const std::string& message)
{
  Tweaklets::Get(MessageDialogTweaklet::KEY)->OpenInformation(parent, title,
      message);
}

bool MessageDialog::OpenQuestion(Shell::Pointer parent,
    const std::string& title, const std::string& message)
{
  return Tweaklets::Get(MessageDialogTweaklet::KEY)->OpenQuestion(parent, title,
      message);
}

void MessageDialog::OpenWarning(Shell::Pointer parent,
    const std::string& title, const std::string& message)
{
  Tweaklets::Get(MessageDialogTweaklet::KEY)->OpenWarning(parent, title,
      message);
}

IDialog::Pointer MessageDialog::CreateMessageDialog(Shell::Pointer parentShell, const std::string& dialogTitle,
         void* dialogTitleImage, const std::string& dialogMessage, int dialogImageType,
         const std::vector<std::string>& dialogButtonLabels, int defaultIndex)
{
  return Tweaklets::Get(MessageDialogTweaklet::KEY)->MessageDialog(parentShell, dialogTitle,
      dialogTitleImage, dialogMessage, dialogImageType,
      dialogButtonLabels, defaultIndex);
}

}
