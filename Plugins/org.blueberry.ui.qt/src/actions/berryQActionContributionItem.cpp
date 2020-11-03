/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "berryQActionContributionItem.h"

#include <berryIContributionManager.h>
#include <berryIContributionManagerOverrides.h>

#include <QMenu>

namespace berry {

QActionContributionItem::QActionContributionItem(QAction* action, const QString& id)
  : ContributionItem(id), action(action)
{
}

bool QActionContributionItem::operator==(const Object* o) const
{
  if (const QActionContributionItem* aci = dynamic_cast<const QActionContributionItem*>(o))
  {
    return action == aci->action;
  }
  return false;
}

uint QActionContributionItem::HashCode() const
{
  return qHash(action);
}

void QActionContributionItem::Fill(QStatusBar* /*parent*/)
{
//  if (widget == null && parent != null) {
//    int flags = SWT.PUSH;
//    if (action != null) {
//      if (action.getStyle() == IAction.AS_CHECK_BOX) {
//        flags = SWT.TOGGLE;
//      }
//      if (action.getStyle() == IAction.AS_RADIO_BUTTON) {
//        flags = SWT.RADIO;
//      }
//    }

//    Button b = new Button(parent, flags);
//    b.setData(this);
//    b.addListener(SWT.Dispose, getButtonListener());
//    // Don't hook a dispose listener on the parent
//    b.addListener(SWT.Selection, getButtonListener());
//    if (action.getHelpListener() != null) {
//      b.addHelpListener(action.getHelpListener());
//    }
//    widget = b;

//    update(null);

//    // Attach some extra listeners.
//    action.addPropertyChangeListener(propertyListener);
//    if (action != null) {
//      String commandId = action.getActionDefinitionId();
//      ExternalActionManager.ICallback callback = ExternalActionManager
//          .getInstance().getCallback();

//      if ((callback != null) && (commandId != null)) {
//        callback.addPropertyChangeListener(commandId,
//            actionTextListener);
//      }
//    }
//  }
}

void QActionContributionItem::Fill(QMenu* parent, QAction* before)
{
  if (parent != nullptr)
  {
    if (before)
    {
      parent->insertAction(before, action);
    }
    else
    {
      parent->addAction(action);
    }
    action->setData(QVariant::fromValue(Object::Pointer(this)));
    action->setProperty("contributionItem", QVariant::fromValue(Object::Pointer(this)));
    //action.addListener(SWT.Dispose, getMenuItemListener());
    //action.addListener(SWT.Selection, getMenuItemListener());
    //if (action.getHelpListener() != null)
    //{
    //  mi.addHelpListener(action.getHelpListener());
    //}

    if (action->menu())
    {
//      // just create a proxy for now, if the user shows it then
//      // fill it in
//      Menu subMenu = new Menu(parent);
//      subMenu.addListener(SWT.Show, getMenuCreatorListener());
//      subMenu.addListener(SWT.Hide, getMenuCreatorListener());
//      mi.setMenu(subMenu);
    }

    Update(QString());

//    // Attach some extra listeners.
//    action.addPropertyChangeListener(propertyListener);
//    if (action != null) {
//      String commandId = action.getActionDefinitionId();
//      ExternalActionManager.ICallback callback = ExternalActionManager
//          .getInstance().getCallback();

//      if ((callback != null) && (commandId != null)) {
//        callback.addPropertyChangeListener(commandId,
//            actionTextListener);
//      }
//    }
  }
}

void QActionContributionItem::Fill(QToolBar* /*parent*/, QAction* /*before*/)
{
//  if (widget == null && parent != null) {
//    int flags = SWT.PUSH;
//    if (action != null) {
//      int style = action.getStyle();
//      if (style == IAction.AS_CHECK_BOX) {
//        flags = SWT.CHECK;
//      } else if (style == IAction.AS_RADIO_BUTTON) {
//        flags = SWT.RADIO;
//      } else if (style == IAction.AS_DROP_DOWN_MENU) {
//        flags = SWT.DROP_DOWN;
//      }
//    }

//    ToolItem ti = null;
//    if (index >= 0) {
//      ti = new ToolItem(parent, flags, index);
//    } else {
//      ti = new ToolItem(parent, flags);
//    }
//    ti.setData(this);
//    ti.addListener(SWT.Selection, getToolItemListener());
//    ti.addListener(SWT.Dispose, getToolItemListener());

//    widget = ti;

//    update(null);

//    // Attach some extra listeners.
//    action.addPropertyChangeListener(propertyListener);
//    if (action != null) {
//      String commandId = action.getActionDefinitionId();
//      ExternalActionManager.ICallback callback = ExternalActionManager
//          .getInstance().getCallback();

//      if ((callback != null) && (commandId != null)) {
//        callback.addPropertyChangeListener(commandId,
//            actionTextListener);
//      }
//    }
//  }
}

QAction* QActionContributionItem::GetAction() const
{
  return action;
}

QActionContributionItem::~QActionContributionItem()
{
  //holdMenu = null;
}

ContributionItem::Modes QActionContributionItem::GetMode() const
{
  return mode;
}

bool QActionContributionItem::IsDynamic() const
{
//  if (qobject_cast<QMenu*>(action->parentWidget()))
//  {
//    // Optimization. Only recreate the item is the check or radio style
//    // has changed.
//    boolean itemIsCheck = (widget.getStyle() & SWT.CHECK) != 0;
//    boolean actionIsCheck = getAction() != null
//        && getAction().getStyle() == IAction.AS_CHECK_BOX;
//    boolean itemIsRadio = (widget.getStyle() & SWT.RADIO) != 0;
//    boolean actionIsRadio = getAction() != null
//        && getAction().getStyle() == IAction.AS_RADIO_BUTTON;
//    return (itemIsCheck != actionIsCheck)
//        || (itemIsRadio != actionIsRadio);
//  }
  return false;
}

bool QActionContributionItem::IsEnabled() const
{
  return action != nullptr && action->isEnabled();
}

bool QActionContributionItem::IsVisible() const
{
  return ContributionItem::IsVisible() && IsCommandActive();
}

void QActionContributionItem::SetMode(Modes mode)
{
  this->mode = mode;
  Update();
}

void QActionContributionItem::Update()
{
  Update(QString());
}

void QActionContributionItem::Update(const QString& /*propertyName*/)
{
//  if (widget != 0)
//  {
//    // determine what to do
//    boolean textChanged = propertyName == null
//        || propertyName.equals(IAction.TEXT);
//    boolean imageChanged = propertyName == null
//        || propertyName.equals(IAction.IMAGE);
//    boolean tooltipTextChanged = propertyName == null
//        || propertyName.equals(IAction.TOOL_TIP_TEXT);
//    boolean enableStateChanged = propertyName == null
//        || propertyName.equals(IAction.ENABLED)
//        || propertyName
//        .equals(IContributionManagerOverrides.P_ENABLED);
//    boolean checkChanged = (action.getStyle() == IAction.AS_CHECK_BOX || action
//                            .getStyle() == IAction.AS_RADIO_BUTTON)
//        && (propertyName == null || propertyName
//            .equals(IAction.CHECKED));

//    if (widget instanceof ToolItem) {
//      ToolItem ti = (ToolItem) widget;
//      String text = action.getText();
//      // the set text is shown only if there is no image or if forced
//      // by MODE_FORCE_TEXT
//      boolean showText = text != null
//          && ((getMode() & MODE_FORCE_TEXT) != 0 || !hasImages(action));

//      // only do the trimming if the text will be used
//      if (showText && text != null) {
//        text = Action.removeAcceleratorText(text);
//        text = Action.removeMnemonics(text);
//      }

//      if (textChanged) {
//        String textToSet = showText ? text : ""; //$NON-NLS-1$
//        boolean rightStyle = (ti.getParent().getStyle() & SWT.RIGHT) != 0;
//        if (rightStyle || !ti.getText().equals(textToSet)) {
//          // In addition to being required to update the text if
//          // it
//          // gets nulled out in the action, this is also a
//          // workaround
//          // for bug 50151: Using SWT.RIGHT on a ToolBar leaves
//          // blank space
//          ti.setText(textToSet);
//        }
//      }

//      if (imageChanged) {
//        // only substitute a missing image if it has no text
//        updateImages(!showText);
//      }

//      if (tooltipTextChanged || textChanged) {
//        String toolTip = action.getToolTipText();
//        if ((toolTip == null) || (toolTip.length() == 0)) {
//          toolTip = text;
//        }

//        ExternalActionManager.ICallback callback = ExternalActionManager
//            .getInstance().getCallback();
//        String commandId = action.getActionDefinitionId();
//        if ((callback != null) && (commandId != null)
//            && (toolTip != null)) {
//          String acceleratorText = callback
//              .getAcceleratorText(commandId);
//          if (acceleratorText != null
//              && acceleratorText.length() != 0) {
//            toolTip = JFaceResources.format(
//                  "Toolbar_Tooltip_Accelerator", //$NON-NLS-1$
//                  new Object[] { toolTip, acceleratorText });
//        }
//      }

//      // if the text is showing, then only set the tooltip if
//      // different
//      if (!showText || toolTip != null && !toolTip.equals(text)) {
//        ti.setToolTipText(toolTip);
//      } else {
//        ti.setToolTipText(null);
//      }
//    }

//    if (enableStateChanged) {
//      boolean shouldBeEnabled = action.isEnabled()
//          && isEnabledAllowed();

//      if (ti.getEnabled() != shouldBeEnabled) {
//        ti.setEnabled(shouldBeEnabled);
//      }
//    }

//    if (checkChanged) {
//      boolean bv = action.isChecked();

//      if (ti.getSelection() != bv) {
//        ti.setSelection(bv);
//      }
//    }
//    return;
//  }

//  if (widget instanceof MenuItem) {
//    MenuItem mi = (MenuItem) widget;

//    if (textChanged) {
//      int accelerator = 0;
//      String acceleratorText = null;
//      IAction updatedAction = getAction();
//      String text = null;
//      accelerator = updatedAction.getAccelerator();
//      ExternalActionManager.ICallback callback = ExternalActionManager
//          .getInstance().getCallback();

//      // Block accelerators that are already in use.
//      if ((accelerator != 0) && (callback != null)
//          && (callback.isAcceleratorInUse(accelerator))) {
//        accelerator = 0;
//      }

//      /*
//       * Process accelerators on GTK in a special way to avoid Bug
//       * 42009. We will override the native input method by
//       * allowing these reserved accelerators to be placed on the
//       * menu. We will only do this for "Ctrl+Shift+[0-9A-FU]".
//       */
//      final String commandId = updatedAction
//          .getActionDefinitionId();
//      if ((Util.isGtk()) && (callback instanceof IBindingManagerCallback)
//          && (commandId != null)) {
//        final IBindingManagerCallback bindingManagerCallback = (IBindingManagerCallback) callback;
//        final IKeyLookup lookup = KeyLookupFactory.getDefault();
//        final TriggerSequence[] triggerSequences = bindingManagerCallback
//            .getActiveBindingsFor(commandId);
//        for (int i = 0; i < triggerSequences.length; i++) {
//          final TriggerSequence triggerSequence = triggerSequences[i];
//          final Trigger[] triggers = triggerSequence
//              .getTriggers();
//          if (triggers.length == 1) {
//            final Trigger trigger = triggers[0];
//            if (trigger instanceof KeyStroke) {
//              final KeyStroke currentKeyStroke = (KeyStroke) trigger;
//              final int currentNaturalKey = currentKeyStroke
//                  .getNaturalKey();
//              if ((currentKeyStroke.getModifierKeys() == (lookup
//                                                          .getCtrl() | lookup.getShift()))
//                  && ((currentNaturalKey >= '0' && currentNaturalKey <= '9')
//                      || (currentNaturalKey >= 'A' && currentNaturalKey <= 'F') || (currentNaturalKey == 'U'))) {
//                accelerator = currentKeyStroke
//                    .getModifierKeys()
//                    | currentNaturalKey;
//                acceleratorText = triggerSequence
//                    .format();
//                break;
//              }
//            }
//          }
//        }
//      }

//      if (accelerator == 0) {
//        if ((callback != null) && (commandId != null)) {
//          acceleratorText = callback
//              .getAcceleratorText(commandId);
//        }
//      }

//      IContributionManagerOverrides overrides = null;

//      if (getParent() != null) {
//        overrides = getParent().getOverrides();
//      }

//      if (overrides != null) {
//        text = getParent().getOverrides().getText(this);
//      }

//      mi.setAccelerator(accelerator);

//      if (text == null) {
//        text = updatedAction.getText();
//      }

//      if (text != null && acceleratorText == null) {
//        // use extracted accelerator text in case accelerator
//        // cannot be fully represented in one int (e.g.
//        // multi-stroke keys)
//        acceleratorText = LegacyActionTools
//            .extractAcceleratorText(text);
//        if (acceleratorText == null && accelerator != 0) {
//          acceleratorText = Action
//              .convertAccelerator(accelerator);
//        }
//      }

//      if (text == null) {
//        text = ""; //$NON-NLS-1$
//      } else {
//        text = Action.removeAcceleratorText(text);
//      }

//      if (acceleratorText == null) {
//        mi.setText(text);
//      } else {
//        mi.setText(text + '\t' + acceleratorText);
//      }
//    }

//    if (imageChanged) {
//      updateImages(false);
//    }

//    if (enableStateChanged) {
//      boolean shouldBeEnabled = action.isEnabled()
//          && isEnabledAllowed();

//      if (mi.getEnabled() != shouldBeEnabled) {
//        mi.setEnabled(shouldBeEnabled);
//      }
//    }

//    if (checkChanged) {
//      boolean bv = action.isChecked();

//      if (mi.getSelection() != bv) {
//        mi.setSelection(bv);
//      }
//    }

//    return;
//  }

//  if (widget instanceof Button) {
//    Button button = (Button) widget;

//    if (imageChanged) {
//      updateImages(false);
//    }

//    if (textChanged) {
//      String text = action.getText();
//      boolean showText = text != null && ((getMode() & MODE_FORCE_TEXT) != 0 || !hasImages(action));
//      // only do the trimming if the text will be used
//      if (showText) {
//        text = Action.removeAcceleratorText(text);
//      }
//      String textToSet = showText ? text : ""; //$NON-NLS-1$
//      button.setText(textToSet);
//    }

//    if (tooltipTextChanged) {
//      button.setToolTipText(action.getToolTipText());
//    }

//    if (enableStateChanged) {
//      boolean shouldBeEnabled = action.isEnabled()
//          && isEnabledAllowed();

//      if (button.getEnabled() != shouldBeEnabled) {
//        button.setEnabled(shouldBeEnabled);
//      }
//    }

//    if (checkChanged) {
//      boolean bv = action.isChecked();

//      if (button.getSelection() != bv) {
//        button.setSelection(bv);
//      }
//    }
//    return;
//  }
//}
}

bool QActionContributionItem::IsEnabledAllowed() const
{
  if (this->GetParent() == nullptr)
  {
    return true;
  }
  int value = GetParent()->GetOverrides()->GetEnabled(this);
  return (value == -1) ? true : value;
}

//QString QActionContributionItem::ShortenText(const QString& textValue, QToolButton* item)
//{
//if (textValue == null) {
//  return null;
//}

//GC gc = new GC(item.getParent());

//int maxWidth = item.getImage().getBounds().width * 4;

//if (gc.textExtent(textValue).x < maxWidth) {
//  gc.dispose();
//  return textValue;
//}

//for (int i = textValue.length(); i > 0; i--) {
//  String test = textValue.substring(0, i);
//  test = test + ellipsis;
//  if (gc.textExtent(test).x < maxWidth) {
//    gc.dispose();
//    return test;
//  }

//}
//gc.dispose();
//// If for some reason we fall through abort
//return textValue;
//}

//Listener QActionContributionItem::GetToolItemListener()
//{
//if (toolItemListener == null) {
//  toolItemListener = new Listener() {
//    public void handleEvent(Event event) {
//      switch (event.type) {
//      case SWT.Dispose:
//        handleWidgetDispose(event);
//        break;
//      case SWT.Selection:
//        Widget ew = event.widget;
//        if (ew != null) {
//          handleWidgetSelection(event, ((ToolItem) ew)
//              .getSelection());
//        }
//        break;
//      }
//    }
//  };
//}
//return toolItemListener;
//}

//void QActionContributionItem::HandleWidgetDispose(Event e)
//{
//// Check if our widget is the one being disposed.
//if (e.widget == widget) {
//  // Dispose of the menu creator.
//  if (action.getStyle() == IAction.AS_DROP_DOWN_MENU
//      && menuCreatorCalled) {
//    IMenuCreator mc = action.getMenuCreator();
//    if (mc != null) {
//      mc.dispose();
//    }
//  }

//  // Unhook all of the listeners.
//  action.removePropertyChangeListener(propertyListener);
//  if (action != null) {
//    String commandId = action.getActionDefinitionId();
//    ExternalActionManager.ICallback callback = ExternalActionManager
//        .getInstance().getCallback();

//    if ((callback != null) && (commandId != null)) {
//      callback.removePropertyChangeListener(commandId,
//          actionTextListener);
//    }
//  }

//  // Clear the widget field.
//  widget = null;

//  disposeOldImages();
//}
//}

//void QActionContributionItem::HandleWidgetSelection(Event e, bool selection)
//{

//Widget item = e.widget;
//if (item != null) {
//  int style = item.getStyle();

//  if ((style & (SWT.TOGGLE | SWT.CHECK)) != 0) {
//    if (action.getStyle() == IAction.AS_CHECK_BOX) {
//      action.setChecked(selection);
//    }
//  } else if ((style & SWT.RADIO) != 0) {
//    if (action.getStyle() == IAction.AS_RADIO_BUTTON) {
//      action.setChecked(selection);
//    }
//  } else if ((style & SWT.DROP_DOWN) != 0) {
//    if (e.detail == 4) { // on drop-down button
//      if (action.getStyle() == IAction.AS_DROP_DOWN_MENU) {
//        IMenuCreator mc = action.getMenuCreator();
//        menuCreatorCalled = true;
//        ToolItem ti = (ToolItem) item;
//        // we create the menu as a sub-menu of "dummy" so that
//        // we can use
//        // it in a cascading menu too.
//        // If created on a SWT control we would get an SWT
//        // error...
//        // Menu dummy= new Menu(ti.getParent());
//        // Menu m= mc.getMenu(dummy);
//        // dummy.dispose();
//        if (mc != null) {
//          Menu m = mc.getMenu(ti.getParent());
//          if (m != null) {
//            // position the menu below the drop down item
//            Point point = ti.getParent().toDisplay(
//                new Point(e.x, e.y));
//            m.setLocation(point.x, point.y); // waiting
//                              // for SWT
//            // 0.42
//            m.setVisible(true);
//            return; // we don't fire the action
//          }
//        }
//      }
//    }
//  }

//  ExternalActionManager.IExecuteCallback callback = null;
//  String actionDefinitionId = action.getActionDefinitionId();
//  if (actionDefinitionId != null) {
//    Object obj = ExternalActionManager.getInstance()
//        .getCallback();
//    if (obj instanceof ExternalActionManager.IExecuteCallback) {
//      callback = (ExternalActionManager.IExecuteCallback) obj;
//    }
//  }

//  // Ensure action is enabled first.
//  // See 1GAN3M6: ITPUI:WINNT - Any IAction in the workbench can be
//  // executed while disabled.
//  if (action.isEnabled()) {
//    boolean trace = Policy.TRACE_ACTIONS;

//    long ms = 0L;
//    if (trace) {
//      ms = System.currentTimeMillis();
//      System.out.println("Running action: " + action.getText()); //$NON-NLS-1$
//    }

//    IPropertyChangeListener resultListener = null;
//    if (callback != null) {
//      resultListener = new IPropertyChangeListener() {
//        public void propertyChange(PropertyChangeEvent event) {
//          // Check on result
//          if (event.getProperty().equals(IAction.RESULT)) {
//            if (event.getNewValue() instanceof Boolean) {
//              result = (Boolean) event.getNewValue();
//            }
//          }
//        }
//      };
//      action.addPropertyChangeListener(resultListener);
//      callback.preExecute(action, e);
//    }

//    action.runWithEvent(e);

//    if (callback != null) {
//      if (result == null || result.equals(Boolean.TRUE)) {
//        callback.postExecuteSuccess(action, Boolean.TRUE);
//      } else {
//        callback.postExecuteFailure(action,
//            new ExecutionException(action.getText()
//                + " returned failure.")); //$NON-NLS-1$
//      }
//    }

//    if (resultListener!=null) {
//      result = null;
//      action.removePropertyChangeListener(resultListener);
//    }
//    if (trace) {
//      System.out.println((System.currentTimeMillis() - ms)
//          + " ms to run action: " + action.getText()); //$NON-NLS-1$
//    }
//  } else {
//    if (callback != null) {
//      callback.notEnabled(action, new NotEnabledException(action
//          .getText()
//          + " is not enabled.")); //$NON-NLS-1$
//    }
//  }
//}
//}

//bool QActionContributionItem::HasImages(Action* actionToCheck) const
//{
//  return actionToCheck.getImageDescriptor() != null
//      || actionToCheck.getHoverImageDescriptor() != null
//      || actionToCheck.getDisabledImageDescriptor() != null;
//}

bool QActionContributionItem::IsCommandActive() const
{
  return action ? action->isVisible() : true;
}

//bool QActionContributionItem::UpdateImages(bool forceImage)
//{

//  ResourceManager parentResourceManager = JFaceResources.getResources();

//  if (widget instanceof ToolItem) {
//    if (USE_COLOR_ICONS) {
//      ImageDescriptor image = action.getHoverImageDescriptor();
//      if (image == null) {
//        image = action.getImageDescriptor();
//      }
//      ImageDescriptor disabledImage = action
//          .getDisabledImageDescriptor();

//      // Make sure there is a valid image.
//      if (image == null && forceImage) {
//        image = ImageDescriptor.getMissingImageDescriptor();
//      }

//      LocalResourceManager localManager = new LocalResourceManager(
//          parentResourceManager);

//      // performance: more efficient in SWT to set disabled and hot
//      // image before regular image
//      ((ToolItem) widget)
//          .setDisabledImage(disabledImage == null ? null
//              : localManager
//                  .createImageWithDefault(disabledImage));
//      ((ToolItem) widget).setImage(image == null ? null
//          : localManager.createImageWithDefault(image));

//      disposeOldImages();
//      imageManager = localManager;

//      return image != null;
//    }
//    ImageDescriptor image = action.getImageDescriptor();
//    ImageDescriptor hoverImage = action.getHoverImageDescriptor();
//    ImageDescriptor disabledImage = action.getDisabledImageDescriptor();

//    // If there is no regular image, but there is a hover image,
//    // convert the hover image to gray and use it as the regular image.
//    if (image == null && hoverImage != null) {
//      image = ImageDescriptor.createWithFlags(action
//          .getHoverImageDescriptor(), SWT.IMAGE_GRAY);
//    } else {
//      // If there is no hover image, use the regular image as the
//      // hover image,
//      // and convert the regular image to gray
//      if (hoverImage == null && image != null) {
//        hoverImage = image;
//        image = ImageDescriptor.createWithFlags(action
//            .getImageDescriptor(), SWT.IMAGE_GRAY);
//      }
//    }

//    // Make sure there is a valid image.
//    if (hoverImage == null && image == null && forceImage) {
//      image = ImageDescriptor.getMissingImageDescriptor();
//    }

//    // Create a local resource manager to remember the images we've
//    // allocated for this tool item
//    LocalResourceManager localManager = new LocalResourceManager(
//        parentResourceManager);

//    // performance: more efficient in SWT to set disabled and hot image
//    // before regular image
//    ((ToolItem) widget).setDisabledImage(disabledImage == null ? null
//        : localManager.createImageWithDefault(disabledImage));
//    ((ToolItem) widget).setHotImage(hoverImage == null ? null
//        : localManager.createImageWithDefault(hoverImage));
//    ((ToolItem) widget).setImage(image == null ? null : localManager
//        .createImageWithDefault(image));

//    // Now that we're no longer referencing the old images, clear them
//    // out.
//    disposeOldImages();
//    imageManager = localManager;

//    return image != null;
//  } else if (widget instanceof Item || widget instanceof Button) {

//    // Use hover image if there is one, otherwise use regular image.
//    ImageDescriptor image = action.getHoverImageDescriptor();
//    if (image == null) {
//      image = action.getImageDescriptor();
//    }
//    // Make sure there is a valid image.
//    if (image == null && forceImage) {
//      image = ImageDescriptor.getMissingImageDescriptor();
//    }

//    // Create a local resource manager to remember the images we've
//    // allocated for this widget
//    LocalResourceManager localManager = new LocalResourceManager(
//        parentResourceManager);

//    if (widget instanceof Item) {
//      ((Item) widget).setImage(image == null ? null : localManager
//          .createImageWithDefault(image));
//    } else if (widget instanceof Button) {
//      ((Button) widget).setImage(image == null ? null : localManager
//          .createImageWithDefault(image));
//    }

//    // Now that we're no longer referencing the old images, clear them
//    // out.
//    disposeOldImages();
//    imageManager = localManager;

//    return image != null;
//  }
//  return false;
//}

//void QActionContributionItem::DisposeOldImages()
//{
//  if (imageManager != null) {
//    imageManager.dispose();
//    imageManager = null;
//  }
//}

//Listener QActionContributionItem::getMenuCreatorListener() {
//  if (menuCreatorListener == null) {
//    menuCreatorListener = new Listener() {
//      public void handleEvent(Event event) {
//        switch (event.type) {
//        case SWT.Show:
//          handleShowProxy((Menu) event.widget);
//          break;
//        case SWT.Hide:
//          handleHideProxy((Menu) event.widget);
//          break;
//        }
//      }
//    };
//  }
//  return menuCreatorListener;
//}

//void QActionContributionItem::HandleShowProxy(QMenu* proxy)
//{
//  proxy.removeListener(SWT.Show, getMenuCreatorListener());
//  IMenuCreator mc = action.getMenuCreator();
//  menuCreatorCalled  = true;
//  if (mc == null) {
//    return;
//  }
//  holdMenu = mc.getMenu(proxy.getParentMenu());
//  if (holdMenu == null) {
//    return;
//  }
//  copyMenu(holdMenu, proxy);
//}

//void QActionContributionItem::CopyMenu(QMenu* realMenu, QMenu* proxy) {
//  if (realMenu.isDisposed() || proxy.isDisposed()) {
//    return;
//  }

//  // we notify the real menu so it can populate itself if it was
//  // listening for SWT.Show
//  realMenu.notifyListeners(SWT.Show, null);

//  final Listener passThrough = new Listener() {
//    public void handleEvent(Event event) {
//      if (!event.widget.isDisposed()) {
//        Widget realItem = (Widget) event.widget.getData();
//        if (!realItem.isDisposed()) {
//          int style = event.widget.getStyle();
//          if (event.type == SWT.Selection
//              && ((style & (SWT.TOGGLE | SWT.CHECK | SWT.RADIO)) != 0)
//              && realItem instanceof MenuItem) {
//            ((MenuItem) realItem)
//                .setSelection(((MenuItem) event.widget)
//                    .getSelection());
//          }
//          event.widget = realItem;
//          realItem.notifyListeners(event.type, event);
//        }
//      }
//    }
//  };

//  MenuItem[] items = realMenu.getItems();
//  for (int i = 0; i < items.length; i++) {
//    final MenuItem realItem = items[i];
//    final MenuItem proxyItem = new MenuItem(proxy, realItem.getStyle());
//    proxyItem.setData(realItem);
//    proxyItem.setAccelerator(realItem.getAccelerator());
//    proxyItem.setEnabled(realItem.getEnabled());
//    proxyItem.setImage(realItem.getImage());
//    proxyItem.setSelection(realItem.getSelection());
//    proxyItem.setText(realItem.getText());

//    // pass through any events
//    proxyItem.addListener(SWT.Selection, passThrough);
//    proxyItem.addListener(SWT.Arm, passThrough);
//    proxyItem.addListener(SWT.Help, passThrough);

//    final Menu itemMenu = realItem.getMenu();
//    if (itemMenu != null) {
//      // create a proxy for any sub menu items
//      final Menu subMenu = new Menu(proxy);
//      subMenu.setData(itemMenu);
//      proxyItem.setMenu(subMenu);
//      subMenu.addListener(SWT.Show, new Listener() {
//        public void handleEvent(Event event) {
//          event.widget.removeListener(SWT.Show, this);
//          if (event.type == SWT.Show) {
//            copyMenu(itemMenu, subMenu);
//          }
//        }
//      });
//      subMenu.addListener(SWT.Help, passThrough);
//      subMenu.addListener(SWT.Hide, passThrough);
//    }
//  }
//}

//void QActionContributionItem::HandleHideProxy(QMenu* proxy)
//{
//  proxy.removeListener(SWT.Hide, getMenuCreatorListener());
//  proxy.getDisplay().asyncExec(new Runnable() {
//    public void run() {
//      if (!proxy.isDisposed()) {
//        MenuItem parentItem = proxy.getParentItem();
//        proxy.dispose();
//        parentItem.setMenu(holdMenu);
//      }
//      if (holdMenu != null && !holdMenu.isDisposed()) {
//        holdMenu.notifyListeners(SWT.Hide, null);
//      }
//      holdMenu = null;
//    }
//  });
//}

}
