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

#ifndef BERRYQTMESSAGEDIALOGTWEAKLET_H_
#define BERRYQTMESSAGEDIALOGTWEAKLET_H_

#include <berryMessageDialogTweaklet.h>

namespace berry
{

class QtMessageDialogTweaklet: public QObject, public MessageDialogTweaklet
{
  Q_OBJECT
  Q_INTERFACES(berry::MessageDialogTweaklet)

public:

  QtMessageDialogTweaklet();
  QtMessageDialogTweaklet(const QtMessageDialogTweaklet& other);

  bool OpenConfirm(Shell::Pointer, const QString& title,
      const QString& message);
  void OpenError(Shell::Pointer, const QString& title,
      const QString& message);
  void OpenInformation(Shell::Pointer, const QString& title,
      const QString& message);
  bool OpenQuestion(Shell::Pointer, const QString& title,
      const QString& message);
  void OpenWarning(Shell::Pointer, const QString& title,
      const QString& message);

  IDialog::Pointer MessageDialog(Shell::Pointer parentShell,
      const QString& dialogTitle, void* dialogTitleImage,
      const QString& dialogMessage, int dialogImageType, const QList<
          QString>& dialogButtonLabels, int defaultIndex);

};

}

#endif /* BERRYQTMESSAGEDIALOGTWEAKLET_H_ */
