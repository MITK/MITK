/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYVIEWREGISTRY_H_
#define BERRYVIEWREGISTRY_H_

#include "berryIViewRegistry.h"
#include "berryIViewCategory.h"
#include "berryIViewDescriptor.h"

#include "berryCategory.h"
#include "berryViewDescriptor.h"
#include "berryViewRegistryReader.h"
#include "berryStickyViewDescriptor.h"

#include "berryIExtensionPoint.h"


namespace berry
{

/**
 * \ingroup org_blueberry_ui_internal
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
  class ViewCategoryProxy : public IViewCategory, public IPluginContribution
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

    /*
     * @see IViewCategory#GetViews()
     */
    QList<IViewDescriptor::Pointer> GetViews() const override;

    /*
     * @see IViewCategory#GetId()
     */
    QString GetId() const override;

    /*
     * @see IViewCategory#GetPath()
     */
    QStringList GetPath() const override;

    /*
     * @see IViewCategory#GetLabel()
     */
    QString GetLabel() const override;

    /*
     * @see IPluginContribution#GetLocalId()
     */
    QString GetLocalId() const override;

    /*
     * @see IPluginContribution#GetPluginId()
     */
    QString GetPluginId() const override;

    /*
     * @see Object#operator==(Object*)
     */
    bool operator==(const Object* o) const override;

    /*
     * @see Object#HashCode()
     */
    uint HashCode() const override;
  };

private:

  static QString EXTENSIONPOINT_UNIQUE_ID;

  /**
   * A set that will only ever contain ViewDescriptors.
   */
  QList<IViewDescriptor::Pointer> views; // = new TreeSet(new Comparator() {
  //    public int compare(Object o1, Object o2) {
  //      String id1 = ((ViewDescriptor) o1).getId();
  //      String id2 = ((ViewDescriptor) o2).getId();
  //
  //      return id1.compareTo(id2);
  //    }});

  QList<IStickyViewDescriptor::Pointer> sticky;

  QList<IViewDescriptorCategoryPtr> categories;

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
  IViewDescriptorCategoryPtr InternalFindCategory(const QString& id);

  SmartPointer<IExtensionPoint> GetExtensionPointFilter();

protected:

  static const QString TAG_DESCRIPTION; // = "description";


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
  IViewDescriptor::Pointer Find(const QString& id) const override;

  /**
   * Find a category with a given name.
   *
   * @param id the id to search for
   * @return the category or <code>null</code>
   */
  IViewCategory::Pointer FindCategory(const QString& id);

  /**
   * Get the list of view categories.
   */
  QList<IViewCategory::Pointer> GetCategories() override;

  /**
   * Get the list of sticky views minus the sticky views which failed the
   * Expressions check.
   */
  QList<IStickyViewDescriptor::Pointer> GetStickyViews() const override;

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
  QList<IViewDescriptor::Pointer> GetViews() const override;

  /**
   * Adds each view in the registry to a particular category.
   * The view category may be defined in xml.  If not, the view is
   * added to the "misc" category.
   */
  void MapViewsToCategories();

  /* (non-Javadoc)
   * @see org.blueberry.core.runtime.dynamicHelpers.IExtensionChangeHandler#addExtension(org.blueberry.core.runtime.dynamicHelpers.IExtensionTracker, org.blueberry.core.runtime.IExtension)
   */
  //void AddExtension(IExtensionTracker tracker, IExtension addedExtension);

};

} // namespace berry

#endif /*BERRYVIEWREGISTRY_H_*/
