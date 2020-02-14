/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  static const QString DEFAULT_PERSPECTIVE_ID;

  berry::WorkbenchWindowAdvisor *CreateWorkbenchWindowAdvisor(berry::IWorkbenchWindowConfigurer::Pointer) override;
  ~CustomViewerWorkbenchAdvisor() override;

  /**
   * Gets the style manager (berry::IQtStyleManager), adds and initializes a Qt-Stylesheet-File (.qss).
   */
  void Initialize(berry::IWorkbenchConfigurer::Pointer) override;

  /**
   * Returns the ID-String of the initial window perspective.
   */
  QString GetInitialWindowPerspectiveId() override;
};
