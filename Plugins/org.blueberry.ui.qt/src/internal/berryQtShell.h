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


#ifndef BERRYQTMAINWINDOWSHELL_H_
#define BERRYQTMAINWINDOWSHELL_H_

#include <berryShell.h>

#include <QWidget>

namespace berry {

class QtAbstractControlWidget;

class QtShell : public Shell
{

public:

  QtShell(QWidget* parent = 0, Qt::WindowFlags flags = 0);

  ~QtShell();

  // berry::Shell
  void SetBounds(const QRect& bounds);
  QRect GetBounds() const;

  void SetLocation(int x, int y);

  QPoint ComputeSize(int wHint, int hHint, bool changed);

  QString GetText() const;
  void SetText(const QString& text);

  bool IsVisible() const;
  void SetVisible(bool visible);

  void SetActive();

  QWidget* GetControl() const;

  void SetImages(const QList<QIcon>& images);

  bool GetMaximized() const;
  bool GetMinimized() const;
  void SetMaximized(bool maximized);
  void SetMinimized(bool minimized);

  void AddShellListener(IShellListener* listener);
  void RemoveShellListener(IShellListener* listener);

  void Open(bool block = false);

  void Close();

  QList<Shell::Pointer> GetShells();

  Qt::WindowFlags GetStyle () const;

  QWidget* GetWidget();

private:

  QWidget* widget;
  bool updatesDisabled;

};

}

#endif /* BERRYQTMAINWINDOWSHELL_H_ */
