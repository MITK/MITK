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

#ifndef BERRYEMPTYPERSPECTIVE_H_
#define BERRYEMPTYPERSPECTIVE_H_

#include <berryIPerspectiveFactory.h>

#include <org_blueberry_uitest_Export.h>

namespace berry
{

/**
 * This perspective is used for testing api.  It defines an initial
 * layout with no parts, just an editor area.
 */
class BERRY_UITEST_EXPORT EmptyPerspective: public QObject, public IPerspectiveFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)

private:

  static QString LastPerspective;

public:

  EmptyPerspective(const EmptyPerspective& other);

  /**
   * The perspective id for the empty perspective.
   */
  static const QString PERSP_ID; // = "org.blueberry.uitest.util.EmptyPerspective";

  /**
   * The perspective id for the second empty perspective.
   */
  static const QString PERSP_ID2; // = "org.blueberry.uitest.util.EmptyPerspective2";


  /**
   * Returns the descriptor for the perspective last opened using this factory.
   *
   * @return the descriptor for the perspective last opened using this factory, or <code>null</code>
   */
  static QString GetLastPerspective();

  /**
   * Sets the descriptor for the perspective last opened using this factory.
   *
   * @param persp the descriptor for the perspective last opened using this factory, or <code>null</code>
   */
  static void SetLastPerspective(const QString& perspId);

  /**
   * Constructs a new Default layout engine.
   */
  EmptyPerspective();

  /**
   * Defines the initial layout for a perspective.
   *
   * Implementors of this method may add additional views to a
   * perspective.  The perspective already contains an editor folder
   * with <code>ID = ILayoutFactory::ID_EDITORS</code>.  Add additional views
   * to the perspective in reference to the editor folder.
   *
   * This method is only called when a new perspective is created.  If
   * an old perspective is restored from a persistence file then
   * this method is not called.
   *
   * @param factory the factory used to add views to the perspective
   */
  void CreateInitialLayout(IPageLayout::Pointer layout);
};

}

#endif /* BERRYEMPTYPERSPECTIVE_H_ */
