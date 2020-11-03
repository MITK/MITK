/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYQTMAINWINDOWSHELL_H_
#define BERRYQTMAINWINDOWSHELL_H_

#include <berryShell.h>

#include <QWidget>

namespace berry {

class QtAbstractControlWidget;

class QtShell : public Shell
{

public:

  QtShell(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);

  ~QtShell() override;

  // berry::Shell
  void SetBounds(const QRect& bounds) override;
  QRect GetBounds() const override;

  void SetLocation(int x, int y) override;

  QPoint ComputeSize(int wHint, int hHint, bool changed) override;

  QString GetText() const override;
  void SetText(const QString& text) override;

  bool IsVisible() const override;
  void SetVisible(bool visible) override;

  void SetActive() override;

  QWidget* GetControl() const override;

  void SetImages(const QList<QIcon>& images) override;

  bool GetMaximized() const override;
  bool GetMinimized() const override;
  void SetMaximized(bool maximized) override;
  void SetMinimized(bool minimized) override;

  void AddShellListener(IShellListener* listener) override;
  void RemoveShellListener(IShellListener* listener) override;

  void Open(bool block = false) override;

  void Close() override;

  QList<Shell::Pointer> GetShells() override;

  Qt::WindowFlags GetStyle () const override;

  QWidget* GetWidget();

private:

  QWidget* widget;
  bool updatesDisabled;

};

}

#endif /* BERRYQTMAINWINDOWSHELL_H_ */
