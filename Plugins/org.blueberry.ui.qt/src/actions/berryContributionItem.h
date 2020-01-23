/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYCONTRIBUTIONITEM_H
#define BERRYCONTRIBUTIONITEM_H

#include "berryIContributionItem.h"

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

/**
 * An abstract base implementation for contribution items.
 */
class BERRY_UI_QT ContributionItem : public IContributionItem
{

public:

  berryObjectMacro(ContributionItem);

  enum Mode {
    DEFAULT = 0x00,

    /**
     * Mode bit: Show text on tool items or buttons, even if an image is
     * present. If this mode bit is not set, text is only shown on tool items if
     * there is no image present.
     */
    MODE_FORCE_TEXT = 0x01
  };
  Q_DECLARE_FLAGS(Modes, Mode)

  enum Style {

    /**
     * A push button tool item or menu item.
     */
    STYLE_PUSH = 0x01,

    /**
     * A checked tool item or menu item.
     */
    STYLE_CHECK = 0x02,

    /**
     * A radio-button style menu item.
     */
    STYLE_RADIO = 0x04,

    /**
     * A ToolBar pulldown item.
     */
    STYLE_PULLDOWN = 0x08
  };

  /**
   * The default implementation of this <code>IContributionItem</code>
   * method does nothing. Subclasses may override.
   */
  void Fill(QStatusBar* parent) override;

  /**
   * The default implementation of this <code>IContributionItem</code>
   * method does nothing. Subclasses may override.
   */
  void Fill(QMenu* menu, QAction* before) override;

  /**
   * The default implementation of this <code>IContributionItem</code>
   * method does nothing. Subclasses may override.
   */
  void Fill(QMenuBar* menu, QAction* before) override;

  /**
   * The default implementation of this <code>IContributionItem</code>
   * method does nothing. Subclasses may override.
   */
  void Fill(QToolBar* parent, QAction* before) override;

  /**
   * The default implementation of this <code>IContributionItem</code>
   * method does nothing. Subclasses may override.
   */
  void SaveWidgetState() override;

  /*
   * Method declared on IContributionItem.
   */
  QString GetId() const override;

  /**
   * Returns the parent contribution manager, or <code>null</code> if this
   * contribution item is not currently added to a contribution manager.
   *
   * @return the parent contribution manager, or <code>null</code>
   */
  IContributionManager *GetParent() const;

  /**
   * The default implementation of this <code>IContributionItem</code>
   * method returns <code>false</code>. Subclasses may override.
   */
  bool IsDirty() const override;

  /**
   * The default implementation of this <code>IContributionItem</code>
   * method returns <code>true</code>. Subclasses may override.
   */
  bool IsEnabled() const override;

  /**
   * The default implementation of this <code>IContributionItem</code>
   * method returns <code>false</code>. Subclasses may override.
   */
  bool IsDynamic() const override;

  /**
   * The default implementation of this <code>IContributionItem</code>
   * method returns <code>false</code>. Subclasses may override.
   */
  bool IsGroupMarker() const override;

  /**
   * The default implementation of this <code>IContributionItem</code>
   * method returns <code>false</code>. Subclasses may override.
   */
  bool IsSeparator() const override;

  /**
   * The default implementation of this <code>IContributionItem</code>
   * method returns the value recorded in an internal state variable,
   * which is <code>true</code> by default. <code>setVisible</code>
   * should be used to change this setting.
   */
  bool IsVisible() const override;

  /**
   * The default implementation of this <code>IContributionItem</code>
   * method stores the value in an internal state variable,
   * which is <code>true</code> by default.
   */
  void SetVisible(bool visible) override;

  /**
   * Returns a string representation of this contribution item
   * suitable only for debugging.
   */
  QString ToString() const override;

  /**
   * The default implementation of this <code>IContributionItem</code>
   * method does nothing. Subclasses may override.
   */
  void Update() override;

  /*
   * Method declared on IContributionItem.
   */
  void SetParent(IContributionManager* parent) override;

  /**
   * The <code>ContributionItem</code> implementation of this
   * method declared on <code>IContributionItem</code> does nothing.
   * Subclasses should override to update their state.
   */
  void Update(const QString& id) override;

  /**
   * The ID for this contribution item. It should be set once either in the
   * constructor or using this method.
   *
   * @param itemId
   * @see #getId()
   */
  void SetId(const QString& itemId);

protected:

  /**
   * Creates a contribution item with a <code>null</code> id.
   * Calls <code>this(String)</code> with <code>null</code>.
   */
  ContributionItem();

  /**
   * Creates a contribution item with the given (optional) id.
   * The given id is used to find items in a contribution manager,
   * and for positioning items relative to other items.
   *
   * @param id the contribution item identifier, or <code>null</code>
   */
  ContributionItem(const QString& id);

private:

  /**
   * The identifier for this contribution item, of <code>null</code> if none.
   */
  QString id;

  /**
   * Indicates this item is visible in its manager; <code>true</code>
   * by default.
   */
  bool visible;

  /**
   * The parent contribution manager for this item
   */
  IContributionManager* parent;
};

}

#endif // BERRYCONTRIBUTIONITEM_H
