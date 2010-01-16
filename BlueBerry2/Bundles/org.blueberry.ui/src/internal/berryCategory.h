/*=========================================================================

Program:   BlueBerry Platform
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

#ifndef BERRYCATEGORY_H_
#define BERRYCATEGORY_H_

#include <berryIAdaptable.h>
#include <osgi/framework/Macros.h>

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
template<class T> class Category : /*IWorkbenchAdapter*/public IAdaptable, public Object
{

public:

  osgiObjectMacro(Category<T>);

  typedef T ElementType;

  /**
   * Name of the miscellaneous category
   */
  const static std::string MISC_NAME;

  /**
   * Identifier of the miscellaneous category
   */
  const static std::string MISC_ID;

private:

  std::string id;
  std::string name;
  std::vector<std::string> parentPath;

  std::vector<ElementType> elements;

  IConfigurationElement::Pointer configurationElement;

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
  Category(const std::string& id, const std::string& label);

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
  SmartPointer<ImageDescriptor> GetImageDescriptor() const;

  /**
   * Return the id for this category.
   * @return the id
   */
  const std::string& GetId() const;

  /**
   * Return the label for this category.
   *
   * @return the label
   */
  std::string GetLabel() const;

  /**
   * Return the parent path for this category.
   *
   * @return the parent path
   */
  const std::vector<std::string>& GetParentPath();

  /**
   * Return the unparsed parent path.  May be <code>null</code>.
   *
   * @return the unparsed parent path or <code>null</code>
   */
  std::string GetRawParentPath() const;

  /**
   * Return the root path for this category.
   *
   * @return the root path
   */
  std::string GetRootPath();

  /**
   * Return the elements contained in this category.
   *
   * @return the elements
   */
  const std::vector<ElementType>& GetElements() const;

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

  /* (non-Javadoc)
   * @see org.blueberry.ui.model.IWorkbenchAdapter#getParent(java.lang.Object)
   */
  ElementType* GetParent(const ElementType& o);

  /**
   * Clear all elements from this category.
   *
   */
  void Clear();

protected:

  /* (non-Javadoc)
   * Method declared on IAdaptable.
   */
  Poco::Any GetAdapter(const std::string& adapter);
};

} // namespace berry


#include "berryCategory.txx"

#endif /*BERRYCATEGORY_H_*/
