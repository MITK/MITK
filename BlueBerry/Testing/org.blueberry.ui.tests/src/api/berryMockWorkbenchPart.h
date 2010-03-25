/*=========================================================================
 
 Program:   BlueBerry Platform
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

#ifndef BERRYWORKBENCHMOCKPART_H_
#define BERRYWORKBENCHMOCKPART_H_

#include <util/berryCallHistory.h>

#include "berryMockSelectionProvider.h"

#include <berryIWorkbenchPart.h>
#include <berryIWorkbenchPartSite.h>
#include <berryIExecutableExtension.h>
#include <berryIConfigurationElement.h>
#include <berryIPropertyChangeListener.h>
#include <berryGuiTkIControlListener.h>

namespace berry
{

/**
 * Base class for mock intro and workbench parts.
 *
 * @since 3.0
 */
class MockWorkbenchPart: public virtual IWorkbenchPart, public IExecutableExtension
{

public:

  berryObjectMacro(MockWorkbenchPart)

  MockWorkbenchPart();

  void SetSite(IWorkbenchPartSite::Pointer site);

  IWorkbenchPartSite::Pointer GetSite() const;

  /**
   * @see IWorkbenchPart#GetPartName()
   */
  std::string GetPartName() const;

  /**
   * @see IWorkbenchPart#GetContentDescription()
   */
  std::string GetContentDescription() const;

  /**
   * @see IWorkbenchPart#getTitleToolTip()
   */
  std::string GetTitleToolTip() const;

  CallHistory::Pointer GetCallHistory() const;

  ISelectionProvider::Pointer GetSelectionProvider();

  void SetInitializationData(IConfigurationElement::Pointer config,
      const std::string& propertyName, Object::Pointer data);

  // This isn't actually part of the part API, but we call this method from a dispose listener
  // in order to mark the point in time at which the widgets are disposed
  void ControlDestroyed(GuiTk::ControlEvent::Pointer e);

  /**
   * @see IWorkbenchPart#addPropertyListener(IPropertyListener)
   */
  void AddPropertyListener(IPropertyChangeListener::Pointer listener);

  /**
   * @see IWorkbenchPart#removePropertyListener(IPropertyListener)
   */
  void RemovePropertyListener(IPropertyChangeListener::Pointer listener);

  std::string GetPartProperty(const std::string& key) const;

  void SetPartProperty(const std::string& key, const std::string& value);

  const std::map<std::string, std::string>& GetPartProperties() const;

  /**
   * @see IWorkbenchPart#createPartControl(Composite)
   */
  void CreatePartControl(void* parent);

  /**
   * @see IWorkbenchPart#dispose()
   */
  ~MockWorkbenchPart();

  /**
   * @see IWorkbenchPart#getTitleImage()
   */
  void* GetTitleImage() const;

  /**
   * @see IWorkbenchPart#setFocus()
   */
  void SetFocus();

  //    /**
  //     * @see IAdaptable#getAdapter(Class)
  //     */
  //    Object::Pointer GetAdapter(Class arg0) {
  //        return null;
  //    }

  /**
   * Fires a selection out.
   */
  void FireSelection();

  /**
   * Gets whether the site was properly initialized in the init method.
   */
  bool IsSiteInitialized();

protected:

  CallHistory::Pointer callTrace;

  MockSelectionProvider::Pointer selectionProvider;

  // IActionBars GetActionBars() = 0;

  void SetSiteInitialized();

  IConfigurationElement::Pointer GetConfig();

  Object::Pointer GetData();

  /**
   * Fires a property change event.
   */
  void FirePropertyChange(int propertyId);

  /**
   * Sets whether the site was properly initialized in the init method.
   */
  void SetSiteInitialized(bool initialized);

private:

  IConfigurationElement::Pointer config;

  Object::Pointer data;

  void* titleImage;

  void* parentWidget;

  IWorkbenchPartSite::Pointer site;

  std::string title;

  IPropertyChangeListener::Events propertyEvent;

  GuiTk::IControlListener::Pointer disposeListener;

  /**
   * boolean to declare whether the site was properly initialized in the init method.
   */
  bool siteState;

  //    /**
  //     * @param actionBars
  //     * @return
  //     */
  //    bool TestActionBars(IActionBars bars) {
  //        return bars != null && bars.getMenuManager() != null
  //                && bars.getToolBarManager() != null
  //                && bars.getStatusLineManager() != null;
  //
  //    }

  std::map<std::string, std::string> properties;

};

}

#endif /* BERRYWORKBENCHMOCKPART_H_ */
