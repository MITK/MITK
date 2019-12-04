/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYLAYOUTPARTSASH_H_
#define BERRYLAYOUTPARTSASH_H_

#include "berryLayoutPart.h"

#include "berryPartSashContainer.h"

#include "guitk/berryGuiTkISelectionListener.h"
#include "presentations/berryIPresentationFactory.h"

#include <QRect>

namespace berry
{


class LayoutPartSash: public LayoutPart
{

  friend class WorkbenchPage;
  friend class PartSashContainer;
  friend class LayoutTreeNode;

public:

  berryObjectMacro(LayoutPartSash);

  LayoutPartSash(PartSashContainer* rootContainer, int style);
  ~LayoutPartSash() override;

private:

  QWidget* sash;
  bool enabled;

  PartSashContainer* rootContainer;

  int style;

  LayoutPartSash::Pointer preLimit;

  LayoutPartSash::Pointer postLimit;

  int left;
  int right;

  QRect bounds;

  IPresentationFactory* presFactory;

  /**
   * Stores whether or not the sash is visible. (This is expected to have a meaningful
   * value even if the underlying control doesn't exist).
   */
  bool isVisible;

  struct SelectionListener : public GuiTk::ISelectionListener
  {
    SelectionListener(LayoutPartSash* layoutPartSash);

    void WidgetSelected(GuiTk::SelectionEvent::Pointer e) override;

  private: LayoutPartSash* layoutPartSash;
  };

  GuiTk::ISelectionListener::Pointer selectionListener;


  // checkDragLimit contains changes by cagatayk@acm.org
  void CheckDragLimit(GuiTk::SelectionEvent::Pointer event);

  /**
   * Creates the control. As an optimization, creation of the control is deferred if
   * the control is invisible.
   */
public:
  void CreateControl(QWidget* parent) override;

  /**
   * Creates the underlying SWT control.
   *
   * @since 3.1
   */
private:
  void DoCreateControl();

public:
  void SetBounds(const QRect& r) override;

  /**
   * Makes the sash visible or invisible. Note: as an optimization, the actual widget is destroyed when the
   * sash is invisible.
   */
public:
  void SetVisible(bool visible) override;

public:
  bool IsVisible() override;

  /**
   * See LayoutPart#dispose
   */
public:
  void Dispose() override;

  /**
   * Gets the presentation bounds.
   */
public:
  QRect GetBounds();

  /**
   * Returns the part control.
   */
public:
  QWidget* GetControl() override;

  /**
   *
   */
public:
  QString GetID() const override;

protected:

  LayoutPartSash::Pointer GetPostLimit();

  LayoutPartSash::Pointer GetPreLimit();

  int GetLeft();

  int GetRight();

  bool IsHorizontal();

  bool IsVertical();

  void SetPostLimit(LayoutPartSash::Pointer newPostLimit);

  void SetPreLimit(LayoutPartSash::Pointer newPreLimit);

  void SetRatio(float newRatio);

  void SetSizes(int left, int right);

private:
  void FlushCache();

private:
  void WidgetSelected(int x, int y, int width, int height);

  /**
   * @param resizable
   * @since 3.1
   */
public:
  void SetEnabled(bool resizable);

protected:

  /* package */int GetSashSize() const;


};

}

#endif /* BERRYLAYOUTPARTSASH_H_ */
