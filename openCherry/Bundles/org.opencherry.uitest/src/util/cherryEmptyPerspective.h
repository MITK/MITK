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

#ifndef CHERRYEMPTYPERSPECTIVE_H_
#define CHERRYEMPTYPERSPECTIVE_H_

#include <cherryIPerspectiveFactory.h>

namespace cherry
{

/**
 * This perspective is used for testing api.  It defines an initial
 * layout with no parts, just an editor area.
 */
class EmptyPerspective: public IPerspectiveFactory
{

private:

  static std::string LastPerspective;

public:

  /**
   * The perspective id for the empty perspective.
   */
  static const std::string PERSP_ID; // = "org.opencherry.uitest.util.EmptyPerspective";

  /**
   * The perspective id for the second empty perspective.
   */
  static const std::string PERSP_ID2; // = "org.opencherry.uitest.util.EmptyPerspective2";


  /**
   * Returns the descriptor for the perspective last opened using this factory.
   *
   * @return the descriptor for the perspective last opened using this factory, or <code>null</code>
   */
  static std::string GetLastPerspective();

  /**
   * Sets the descriptor for the perspective last opened using this factory.
   *
   * @param persp the descriptor for the perspective last opened using this factory, or <code>null</code>
   */
  static void SetLastPerspective(const std::string& perspId);

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

#endif /* CHERRYEMPTYPERSPECTIVE_H_ */
