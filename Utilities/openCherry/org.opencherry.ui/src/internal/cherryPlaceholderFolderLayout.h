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

#ifndef CHERRYPLACEHOLDERFOLDERLAYOUT_H_
#define CHERRYPLACEHOLDERFOLDERLAYOUT_H_

#include "../cherryIPlaceholderFolderLayout.h"

#include "cherryLayoutPart.h"
#include "cherryPageLayout.h"

namespace cherry
{

/**
 * \ingroup org_opencherry_ui_internal
 * 
 * This layout is used to define the initial set of placeholders
 * in a placeholder.
 * <p>
 * Views are added to the placeholder by ID. This id is used to identify 
 * a view descriptor in the view registry, and this descriptor is used to 
 * instantiate the IViewPart.
 * </p>
 */
class PlaceholderFolderLayout : public IPlaceholderFolderLayout
{

public:
  cherryClassMacro(PlaceholderFolderLayout)

private:

  PageLayout::Pointer pageLayout;

  ContainerPlaceholder::Pointer placeholder;

public:

  PlaceholderFolderLayout(PageLayout::Pointer pageLayout,
      ContainerPlaceholder::Pointer folder);

  /**
   * @see IPlaceholderFolderLayout
   */
  void AddPlaceholder(const std::string& viewId);

  /* (non-Javadoc)
   * @see org.eclipse.ui.IPlaceholderFolderLayout#getProperty(java.lang.String)
   */
  std::string GetProperty(const std::string& id);

  /* (non-Javadoc)
   * @see org.eclipse.ui.IPlaceholderFolderLayout#setProperty(java.lang.String, java.lang.String)
   */
  void SetProperty(const std::string& id, const std::string& value);

private:

  /**
   * Inform the page layout of the new part created
   * and the placeholder the part belongs to.
   */
  void LinkPartToPageLayout(const std::string& viewId,
      LayoutPart::Pointer newPart);

};

}

#endif /*CHERRYPLACEHOLDERFOLDERLAYOUT_H_*/
