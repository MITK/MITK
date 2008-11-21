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

#ifndef CHERRYLAYOUTPARTSASH_H_
#define CHERRYLAYOUTPARTSASH_H_

#include "cherryLayoutPart.h"

#include "cherryPartSashContainer.h"

#include "../guitk/cherryGuiTkISelectionListener.h"
#include "../presentations/cherryIPresentationFactory.h"

namespace cherry
{


class LayoutPartSash: public LayoutPart
{

  friend class WorkbenchPage;
  friend class PartSashContainer;
  friend class LayoutTreeNode;

public:

  cherryClassMacro(LayoutPartSash);

  LayoutPartSash(PartSashContainer* rootContainer, int style);
  ~LayoutPartSash();

private:

  void* sash;
  bool enabled;

  PartSashContainer* rootContainer;

  int style;

  LayoutPartSash::Pointer preLimit;

  LayoutPartSash::Pointer postLimit;

  int left;
  int right;

  Rectangle bounds;

  IPresentationFactory* presFactory;

  /**
   * Stores whether or not the sash is visible. (This is expected to have a meaningful
   * value even if the underlying control doesn't exist).
   */
  bool isVisible;

  struct SelectionListener : public GuiTk::ISelectionListener
  {
    SelectionListener(LayoutPartSash* layoutPartSash);

    void WidgetSelected(GuiTk::SelectionEvent::Pointer e);

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
  void CreateControl(void* parent);

  /**
   * Creates the underlying SWT control.
   *
   * @since 3.1
   */
private:
  void DoCreateControl();

public:
  void SetBounds(const Rectangle& r);

  /**
   * Makes the sash visible or invisible. Note: as an optimization, the actual widget is destroyed when the
   * sash is invisible.
   */
public:
  void SetVisible(bool visible);

public:
  bool IsVisible();

  /**
   * See LayoutPart#dispose
   */
public:
  void Dispose();

  /**
   * Gets the presentation bounds.
   */
public:
  Rectangle GetBounds();

  /**
   * Returns the part control.
   */
public:
  void* GetControl();

  /**
   *
   */
public:
  std::string GetID();

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

#endif /* CHERRYLAYOUTPARTSASH_H_ */
