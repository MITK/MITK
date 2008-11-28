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

#include <QMainWindow>

namespace cherry {

class QtMainWindowShell : public QMainWindow, public Shell
{
  Q_OBJECT

private:

  std::list<IShellListener::Pointer> shellListeners;

  typedef std::list<GuiTk::IControlListener::Pointer> ControlListenersList;
  ControlListenersList controlListeners;

public:

  QtMainWindowShell(QWidget* parent = 0, Qt::WindowFlags flags = 0);

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

protected:

  // QMainWindow

  void changeEvent(QEvent* event);

  void closeEvent(QCloseEvent* closeEvent);

  void moveEvent(QMoveEvent* event);
  void resizeEvent(QResizeEvent* event);
  void inFocusEvent(QFocusEvent* event);

};

}

#endif /* CHERRYQTMAINWINDOWSHELL_H_ */
