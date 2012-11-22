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


#ifndef VIEWERPERSPECTIVE_H_
#define VIEWERPERSPECTIVE_H_

#include <berryIPerspectiveFactory.h>

#include <QObject>

/**
 * \brief Perspective class used as viewer perspective within the custom viewer plug-in.
 *
 * This is the perspective class that is used as viewer perspective within the custom viewer
 * plug-in. The ViewerPerspective holds an instance of each the QmitkDataManagerView and the
 * SimpleRenderWindowView. The editor area is made invisible, and QmitkDataManagerView and
 * the SimpleRenderWindowView instances are integrated as non-closeable fixated standalone views.
 */
// //! [ViewerPerspectiveClassDeclaration]
class ViewerPerspective : public QObject, public berry::IPerspectiveFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)

public:

  ViewerPerspective();

  /**
  * An instance of each the QmitkDataManagerView and the SimpleRenderWindowView are
  * initially being laid out.
  */
  void CreateInitialLayout(berry::IPageLayout::Pointer layout);

};
// //! [ViewerPerspectiveClassDeclaration]

#endif /* VIEWERPERSPECTIVE_H_ */
