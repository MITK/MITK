/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYCATEGORY_H_
#define BERRYCATEGORY_H_

#include <berryIAdaptable.h>
#include <berryIPluginContribution.h>

namespace berry
{

/**
 * \ingroup org_blueberry_ui_internal
 *
 * Category provides for hierarchical grouping of elements
 * registered in the registry. One extension normally defines
 * a category, and other reference it via its ID.
 * <p>
 * A category may specify its parent category in order to
 * achieve hierarchy.
 * </p>
 */
template<class T> class Category : /*IWorkbenchAdapter*/ public IPluginContribution, public IAdaptable
{

public:

  berryObjectMacro(Category<T>, IPluginContribution, IAdaptable);

  typedef T ElementType;

  /**
   * Name of the miscellaneous category
   */
  const static QString MISC_NAME;

  /**
   * Identifier of the miscellaneous category
   */
  const static QString MISC_ID;

private:

  QString id;
  QString name;
  QList<QString> parentPath;

  QList<ElementType> elements;

  IConfigurationElement::Pointer configurationElement;

  QString pluginId;

public:

  /**
   * Creates an instance of <code>Category</code> as a
   * miscellaneous category.
   */
  Category();

  /**
   * Creates an instance of <code>Category</code> with
   * an ID and label.
   *
   * @param id the unique identifier for the category
   * @param label the presentation label for this category
   */
  Category(const QString& id, const QString& label);

  /**
   * Creates an instance of <code>Category</code> using the
   * information from the specified configuration element.
   *
   * @param configElement the <code>IConfigurationElement<code> containing
   *    the ID, label, and optional parent category path.
   * @throws WorkbenchException if the ID or label is <code>null</code
   */
  Category(IConfigurationElement::Pointer configElement);

  /**
   * Add an element to this category.
   *
   * @param element the element to add
   */
  void AddElement(ElementType element);

  /* (non-Javadoc)
   * Method declared on IWorkbenchAdapter.
   */
  QIcon GetImageDescriptor() const;

  /**
   * Return the id for this category.
   * @return the id
   */
  const QString& GetId() const;

  /**
   * Return the label for this category.
   *
   * @return the label
   */
  QString GetLabel() const;

  /**
   * Return the parent path for this category.
   *
   * @return the parent path
   */
  QList<QString> GetParentPath();

  /**
   * Return the unparsed parent path.  May be <code>null</code>.
   *
   * @return the unparsed parent path or <code>null</code>
   */
  QString GetRawParentPath() const;

  /**
   * Return the root path for this category.
   *
   * @return the root path
   */
  QString GetRootPath();

  /**
   * Return the elements contained in this category.
   *
   * @return the elements
   */
  const QList<ElementType>& GetElements() const;

  /**
   * Return whether a given object exists in this category.
   *
   * @param o the object to search for
   * @return whether the object is in this category
   */
  bool HasElement(const ElementType& o) const;

  /**
   * Return whether this category has child elements.
   *
   * @return whether this category has child elements
   */
  bool HasElements() const;

  /*
   * @see IWorkbenchAdapter#GetParent(Object)
   */
  ElementType* GetParent(const ElementType& o);

  /*
   * @see IPluginContribution#GetLocalId()
   */
  QString GetLocalId() const override;

  /*
   * @see IPluginContribution#GetPluginId()
   */
  QString GetPluginId() const override;

  /**
   * Clear all elements from this category.
   *
   */
  void Clear();

protected:

  /* (non-Javadoc)
   * Method declared on IAdaptable.
   */
  Object* GetAdapter(const QString& adapter) const override;
};

} // namespace berry


#include "berryCategory.txx"

#endif /*BERRYCATEGORY_H_*/
