/*=========================================================================

 Program:   openCherry Platform
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


#ifndef CHERRYQTMAINWINDOWSHELL_H_
#define CHERRYQTMAINWINDOWSHELL_H_

#include <cherryShell.h>
#include <cherryPoint.h>

#include <QWidget>

namespace cherry {

class QtAbstractControlWidget;

class QtShell : public Shell
{

public:

  QtShell(QWidget* parent = 0, Qt::WindowFlags flags = 0);

  ~QtShell();

  // cherry::Shell
  void SetBounds(const Rectangle& bounds);
  Rectangle GetBounds() const;

  void SetLocation(int x, int y);

  Point ComputeSize(int wHint, int hHint, bool changed);

  void SetText(const std::string& text);

  bool IsVisible();
  void SetVisible(bool visible);

  void* GetControl();

  void SetImages(const std::vector<void*>& images);

  bool GetMaximized();
  bool GetMinimized();
  void SetMaximized(bool maximized);
  void SetMinimized(bool minimized);

  void AddShellListener(IShellListener::Pointer listener);
  void RemoveShellListener(IShellListener::Pointer listener);

  void AddControlListener(GuiTk::IControlListener::Pointer listener);
  void RemoveControlListener(GuiTk::IControlListener::Pointer listener);

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

#endif /* CHERRYQTMAINWINDOWSHELL_H_ */
