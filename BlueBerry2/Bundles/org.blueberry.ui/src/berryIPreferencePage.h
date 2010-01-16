/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date: 2009-01-23 09:44:29 +0100 (Fr, 23 Jan 2009) $
 Version:   $Revision: 16084 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#ifndef BERRYIPREFERENCEPAGE_H_
#define BERRYIPREFERENCEPAGE_H_

#include "berryObject.h"
#include "berryIPreferences.h"
#include "berryIWorkbench.h"

namespace berry
{

/**
 * \ingroup org_blueberry_ui
 *
 * Interface for workbench preference pages.
 * <p>
 * Clients should implement this interface and include the name of their class
 * in an extension contributed to the workbench's preference extension point
 * (named <code>"org.blueberry.ui.preferencePages"</code>).
 * For example, the plug-in's XML markup might contain:
 * <pre>
 * &LT;extension point="org.blueberry.ui.preferencePages"&GT;
 *      &LT;page id="com.example.myplugin.prefs"
 *         name="Knobs"
 *         class="ns::MyPreferencePage" /&GT;
 * &LT;/extension&GT;
 * </pre>
 * </p>
 */
struct IPreferencePage: virtual public Object
{

  berryInterfaceMacro(IPreferencePage, berry)

  /**
     * Initializes this preference page for the given workbench.
     * <p>
     * This method is called automatically as the preference page is being created
     * and initialized. Clients must not call this method.
     * </p>
     *
     * @param workbench the workbench
     */
   virtual void Init(IWorkbench::Pointer workbench) = 0;

  /**
   * Creates the top level control for this preference
   * page under the given parent widget.
   * <p>
   * Implementors are responsible for ensuring that
   * the created control can be accessed via <code>GetControl</code>
   * </p>
   *
   * @param parent the parent widget
   */
  virtual void CreateControl(void* parent) = 0;

  /**
   * Returns the top level control for this dialog page.
   * <p>
   * May return <code>null</code> if the control
   * has not been created yet.
   * </p>
   *
   * @return the top level control or <code>null</code>
   */
  virtual void* GetControl() const = 0;

  ///
  /// Invoked when the OK button was clicked in the preferences dialog
  ///
  virtual bool PerformOk() = 0;

  ///
  /// Invoked when the Cancel button was clicked in the preferences dialog
  ///
  virtual void PerformCancel() = 0;

  ///
  /// Invoked when the user performed an import. As the values of the preferences may have changed
  /// you should read all values again from the preferences service.
  ///
  virtual void Update() = 0;
};

}

#endif /*BERRYIPREFERENCEPAGE_H_*/
