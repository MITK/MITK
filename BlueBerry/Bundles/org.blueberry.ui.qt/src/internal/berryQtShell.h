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
#include <berryPoint.h>

#include <QWidget>

namespace berry {

class QtAbstractControlWidget;

class QtShell : public Shell
{

public:

  QtShell(QWidget* parent = 0, Qt::WindowFlags flags = 0);

  ~QtShell();

  // berry::Shell
  void SetBounds(const Rectangle& bounds);
  Rectangle GetBounds() const;

  void SetLocation(int x, int y);

  Point ComputeSize(int wHint, int hHint, bool changed);

  std::string GetText() const;
  void SetText(const std::string& text);

  bool IsVisible();
  void SetVisible(bool visible);

  void SetActive();

  void* GetControl();

  void SetImages(const std::vector<void*>& images);

  bool GetMaximized();
  bool GetMinimized();
  void SetMaximized(bool maximized);
  void SetMinimized(bool minimized);

  void AddShellListener(IShellListener::Pointer listener);
  void RemoveShellListener(IShellListener::Pointer listener);

  void Open(bool block = false);

  void Close();

  std::vector<Shell::Pointer> GetShells();

  int GetStyle ();

  QWidget* GetWidget();

private:

  QWidget* widget;
  bool updatesDisabled;

};

}

#endif /* BERRYQTMAINWINDOWSHELL_H_ */
