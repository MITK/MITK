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

#ifndef CHERRYVIEWREGISTRY_H_
#define CHERRYVIEWREGISTRY_H_

#include "../cherryIViewRegistry.h"
#include "../cherryIViewCategory.h"
#include "../cherryIViewDescriptor.h"

#include "cherryCategory.h"
#include "cherryViewDescriptor.h"
#include "cherryViewRegistryReader.h"
#include "cherryStickyViewDescriptor.h"

#include "service/cherryIExtensionPoint.h"

#include <vector>
#include <string>
#include <set>

namespace cherry
{

/**
 * \ingroup org_opencherry_ui_internal
 *
 * The central manager for view descriptors.
 */
class ViewRegistry : public IViewRegistry
{

public:

  typedef Category<IViewDescriptor::Pointer> IViewDescriptorCategory;
  typedef IViewDescriptorCategory::Pointer IViewDescriptorCategoryPtr;

private:

  /**
   * Proxies a Category implementation.
   *
   */
  class ViewCategoryProxy : public IViewCategory
  {

  private:

    typedef Category<IViewDescriptor::Pointer> IViewDescriptorCategory;

    IViewDescriptorCategoryPtr rawCategory;

    /**
     * Create a new instance of this class
     *
     * @param rawCategory the category
     */
  public:
    ViewCategoryProxy(IViewDescriptorCategoryPtr rawCategory);

    /* (non-Javadoc)
     * @see org.opencherry.ui.views.IViewCategory#getViews()
     */
    const std::vector<IViewDescriptor::Pointer>& GetViews() const;

    /* (non-Javadoc)
     * @see org.opencherry.ui.views.IViewCategory#getId()
     */
    const std::string& GetId() const;

    /* (non-Javadoc)
     * @see org.opencherry.ui.views.IViewCategory#getPath()
     */
    std::vector<std::string> GetPath() const;

    /* (non-Javadoc)
     * @see org.opencherry.ui.views.IViewCategory#getLabel()
     */
    std::string GetLabel() const;

    /* (non-Javadoc)
     * @see java.lang.Object#equals(java.lang.Object)
     */
    bool operator==(const Object* o) const;

    /* (non-Javadoc)
     * @see java.lang.Object#hashCode()
     */
    int HashCode();
  };

private:

  static std::string EXTENSIONPOINT_UNIQUE_ID;

  /**
   * A set that will only ever contain ViewDescriptors.
   */
  std::vector<IViewDescriptor::Pointer> views; // = new TreeSet(new Comparator() {
  //    public int compare(Object o1, Object o2) {
  //      String id1 = ((ViewDescriptor) o1).getId();
  //      String id2 = ((ViewDescriptor) o2).getId();
  //
  //      return id1.compareTo(id2);
  //    }});

  std::vector<IStickyViewDescriptor::Pointer> sticky;

  std::vector<IViewDescriptorCategoryPtr> categories;

  IViewDescriptorCategoryPtr miscCategory;

  ViewRegistryReader reader;

  bool dirtyViewCategoryMappings;

  /**
   * Returns the category with no updating of the view/category mappings.
   *
   * @param id the category id
   * @return the Category
   * @since 3.1
   */
  IViewDescriptorCategoryPtr InternalFindCategory(const std::string& id);

  const IExtensionPoint* GetExtensionPointFilter();

protected:

  static const std::string TAG_DESCRIPTION; // = "description";


public:

  ViewRegistry();

  /**
   * Add a category to the registry.
   *
   * @param desc the descriptor to add
   */
  void Add(IViewDescriptorCategoryPtr desc);

  /**
   * Add a descriptor to the registry.
   *
   * @param desc the descriptor to add
   */
  void Add(ViewDescriptor::Pointer desc);

  /**
   * Add a sticky descriptor to the registry.
   *
   * @param desc the descriptor to add
   */
  void Add(StickyViewDescriptor::Pointer desc);

  /**
   * Find a descriptor in the registry.
   */
  IViewDescriptor::Pointer Find(const std::string& id) const;

  /**
   * Find a category with a given name.
   *
   * @param id the id to search for
   * @return the category or <code>null</code>
   */
  IViewCategory::Pointer FindCategory(const std::string& id);

  /**
   * Get the list of view categories.
   */
  std::vector<IViewCategory::Pointer> GetCategories();

  /**
   * Get the list of sticky views minus the sticky views which failed the
   * Expressions check.
   */
  std::vector<IStickyViewDescriptor::Pointer> GetStickyViews() const;

  /**
   * Returns the Misc category. This may be <code>null</code> if there are
   * no miscellaneous views.
   *
   * @return the misc category or <code>null</code>
   */
  IViewDescriptorCategoryPtr GetMiscCategory() const;

  /**
   * Get an enumeration of view descriptors.
   */
  const std::vector<IViewDescriptor::Pointer>& GetViews() const;

  /**
   * Adds each view in the registry to a particular category.
   * The view category may be defined in xml.  If not, the view is
   * added to the "misc" category.
   */
  void MapViewsToCategories();

  /* (non-Javadoc)
   * @see org.opencherry.core.runtime.dynamicHelpers.IExtensionChangeHandler#addExtension(org.opencherry.core.runtime.dynamicHelpers.IExtensionTracker, org.opencherry.core.runtime.IExtension)
   */
  //void AddExtension(IExtensionTracker tracker, IExtension addedExtension);

};

} // namespace cherry

#endif /*CHERRYVIEWREGISTRY_H_*/
