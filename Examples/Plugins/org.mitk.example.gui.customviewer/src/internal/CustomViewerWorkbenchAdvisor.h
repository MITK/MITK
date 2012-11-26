/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <berryQtWorkbenchAdvisor.h>

/**
 * \brief A WorkbenchAdvisor class for the custom viewer plug-in.
 *
 * This WorkbenchAdvisor class for the custom viewer plug-in adds and sets a Qt-Stylesheet
 * file to the berry::QtStyleManager during the initialization phase for customization purpose.
 */

// //! [WorkbenchAdvisorDecl]
class CustomViewerWorkbenchAdvisor : public berry::QtWorkbenchAdvisor
// //! [WorkbenchAdvisorDecl]
{

public:

  /**
   * Holds the ID-String of the initial window perspective.
   */
  static const std::string DEFAULT_PERSPECTIVE_ID;

  berry::WorkbenchWindowAdvisor* CreateWorkbenchWindowAdvisor(berry::IWorkbenchWindowConfigurer::Pointer);
  ~CustomViewerWorkbenchAdvisor();

  /**
   * Gets the style manager (berry::IQtStyleManager), adds and initializes a Qt-Stylesheet-File (.qss).
   */
  void Initialize(berry::IWorkbenchConfigurer::Pointer);

  /**
   * Returns the ID-String of the initial window perspective.
   */
  std::string GetInitialWindowPerspectiveId();

private:

  QScopedPointer<berry::WorkbenchWindowAdvisor> wwAdvisor;

};
