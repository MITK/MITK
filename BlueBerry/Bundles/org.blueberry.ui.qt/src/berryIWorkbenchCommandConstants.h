/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYIWORKBENCHCOMMANDCONSTANTS_H
#define BERRYIWORKBENCHCOMMANDCONSTANTS_H

#include <QString>

namespace berry {

/**
 * Constants for all commands defined by the BlueBerry workbench.
 *
 * @noextend This interface is not intended to be extended by clients.
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct IWorkbenchCommandConstants {

  // Views Category:

  /**
   * Id for command "Show View" in category "Views"
   * (value is <code>"org.blueberry.ui.views.showView"</code>).
   */
  static QString VIEWS_SHOW_VIEW; // = "org.blueberry.ui.views.showView";

   /**
    * Id for parameter "View Id" in command "Show View" in category "Views"
    * (value is <code>"org.blueberry.ui.views.showView.viewId"</code>).
    */
   static QString VIEWS_SHOW_VIEW_PARM_ID; // = "org.blueberry.ui.views.showView.viewId";

   /**
    * Id for parameter "Secondary Id" in command "Show View" in category "Views"
    * (value is <code>"org.blueberry.ui.views.showView.secondaryId"</code>).
    */
   static QString VIEWS_SHOW_VIEW_SECONDARY_ID; // = "org.blueberry.ui.views.showView.secondaryId";

   /**
    * Id for parameter "As Fastview" in command "Show View" in category "Views"
    * (value is <code>"org.blueberry.ui.views.showView.makeFast"</code>).
    * Optional.
    */
   static QString VIEWS_SHOW_VIEW_PARM_FASTVIEW; // = "org.blueberry.ui.views.showView.makeFast";

   // Perspectives Category:

   /**
    * Id for command "Show Perspective" in category "Perspectives"
    * (value is <code>"org.blueberry.ui.perspectives.showPerspective"</code>).
    */
   static QString PERSPECTIVES_SHOW_PERSPECTIVE; // = "org.blueberry.ui.perspectives.showPerspective";

   /**
    * Id for parameter "Perspective Id" in command "Show Perspective" in
    * category "Perspectives" (value is
    * <code>"org.blueberry.ui.perspectives.showPerspective.perspectiveId"</code>
    * ).
    */
   static QString PERSPECTIVES_SHOW_PERSPECTIVE_PARM_ID; // = "org.blueberry.ui.perspectives.showPerspective.perspectiveId";

   /**
    * Id for parameter "In New Window" in command "Show Perspective" in
    * category "Perspectives" (value is
    * <code>"org.blueberry.ui.perspectives.showPerspective.newWindow"</code>).
    * Optional.
    */
   static QString PERSPECTIVES_SHOW_PERSPECTIVE_PARM_NEWWINDOW; // = "org.blueberry.ui.perspectives.showPerspective.newWindow";

};

}

#endif // BERRYIWORKBENCHCOMMANDCONSTANTS_H
