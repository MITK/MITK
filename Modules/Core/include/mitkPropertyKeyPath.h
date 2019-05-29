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

#ifndef mitkPropertyKeyPath_h
#define mitkPropertyKeyPath_h

#include <string>
#include <vector>

#include <mitkExceptionMacro.h>

#include <MitkCoreExports.h>

namespace mitk
{
  /** @brief Class that can be used to specify nested or wild carded property keys. E.g.
   * for the use in context of the property persistence service or the property relation service.\n
   * Following assumptions are made /preconditions are defined:
   * - A property key is partitioned by "." into nodes (c.f. visualization of property keys in the PropertyView).
   * - A node can either be an element or a selection.
   * - An element has a name (alphanumric, - and space; "A-Za-z0-9- ") or is wildcarded ("*")
   * - A selection is either an index (e.g. "[1]") or a wildcard ("[*]").
   *
   * Selections are used to indicate that the preceding element has multiple occurences and which occurence is meant.
   * Example property keys would be:
   * - prop : A simple property key
   * - prop.subprop1 : A property key consting of two nodes
   * - prop.* : Any property key that starts with a node "prop"
   * - prop.sub.[2] : A property key that starts with a node "prop" and a has a second node that is selection and has
   * the index 2.
   * - prop.sub.[*] : Any property key that starts with a node "prop" and a has a second node that is selection (with
   * any index).
   *
   * To build a path one may use the Add* method to build up the PropertyKeyPath element by element.\n
   * "first.*.third.[3]" would be equivalent to
   * propKeyPath.AddElement("first");
   * propKeyPath.AddAnyElement();
   * propKeyPath.AddSelection("third",3);\n
   * or the inline version
   * propKeyPath.AddElement("first").AddAnyElement().AddSelection("third",3);
   */
  class MITKCORE_EXPORT PropertyKeyPath final
  {
  public:
    using ItemSelectionIndex = std::size_t;
    using ElementNameType = std::string;

    struct MITKCORE_EXPORT NodeInfo
    {
      enum class NodeType
      {
        Invalid = 0,      //*< Node does not exist or is invalid.
        Element,          //*< Selects an specific element given the node name.
        ElementSelection, //*< Selects an specific item in a sequence of items and has a item selector ("[n]").
        AnySelection,     //*< Selects all items of a specific element ("[*]").
        AnyElement //*< Selects any element/item. Node name is wildcarded ("*"); item selection as well implictily.
      };

      NodeType type;
      ElementNameType name;
      ItemSelectionIndex selection;

      NodeInfo();
      NodeInfo(const ElementNameType &name, NodeType type = NodeType::Element, ItemSelectionIndex index = 0);
      bool Matches(const NodeInfo &right) const;

      bool operator==(const NodeInfo &right) const;
    };

    using NodeInfoVectorType = std::vector<NodeInfo>;
    using PathIndexType = NodeInfoVectorType::size_type;

    /** Returns if the PropertyKeyPath is empty.*/
    bool IsEmpty() const;

    /** Returns if the path is explicit (has no wildcards).*/
    bool IsExplicit() const;

    /** Returns if the path has any nodes with item selection wild cards ([*]).*/
    bool HasItemSelectionWildcardsOnly() const;

    /** Number of path nodes the PropertyKeyPath contains.*/
    PathIndexType GetSize() const;

    /** Adds a new node to the end of the path.
    \param [in] newNode Reference to the node that should be added.
    \return Returns the index of the newly added node.*/
    PathIndexType AddNode(const NodeInfo &newNode);

    /** Function returns the node info of a path node specified by the index
     * within the PropertyKeyPath.
     * \pre Passed index must not be out of bounds.
     * \param [in] index Index of the node whose info should be retrieved.
     * \return Info of the specified path node. If the index is out of bound an InvalidPathNode exception will be
     * thrown.*/
    const NodeInfo &GetNode(const PathIndexType &index) const;

    /** Function returns the node info of a path node specified by the index
     * within the PropertyKeyPath.
     * \pre Passed index must not be out of bounds.
     * \param [in] index Index of the node whose info should be retrieved.
     * \return Info of the specified path node. If the index is out of bound an InvalidPathNode exception will be
     * thrown.*/
    NodeInfo &GetNode(const PathIndexType &index);

    /** Function returns the node info of the first path node within the PropertyKeyPath.
     * \pre PropertyKeyPath must not be empty.
     * \return Info of the first path node. If the path is empty, an InvalidPathNode exception will be thrown.*/
    NodeInfo &GetFirstNode();

    /** Function returns the node info of the first path node within the PropertyKeyPath.
     * \pre PropertyKeyPath must not be empty.
     * \return Info of the first path node. If the path is empty, an InvalidPathNode exception will be thrown.*/
    const NodeInfo &GetFirstNode() const;

    /** Function returns the node info of the last path node within the PropertyKeyPath.
     * \pre PropertyKeyPath must not be empty.
     * \return Info of the first path node. If the path is empty, an InvalidPathNode exception will be thrown.*/
    NodeInfo &GetLastNode();

    /** Function returns the node info of the last path node within the PropertyKeyPath.
     * \pre PropertyKeyPath must not be empty.
     * \return Info of the first path node. If the path is empty, an InvalidPathNode exception will be thrown.*/
    const NodeInfo &GetLastNode() const;

    const NodeInfoVectorType &GetNodes() const;

    /**Compares two PropertyKeyPaths for real equality. So it is a string comparison of their string conversion.*/
    bool operator==(const PropertyKeyPath &path) const;

    /**Operation equals like comparing the ToStr() results with operator <.*/
    bool operator<(const PropertyKeyPath &right) const;

    /**Operation equals like comparing the ToStr() results with operator <=.*/
    bool operator<=(const PropertyKeyPath &right) const;

    /**Operation equals like comparing the ToStr() results with operator >=.*/
    bool operator>=(const PropertyKeyPath &right) const;

    /**Operation equals like comparing the ToStr() results with operator >.*/
    bool operator>(const PropertyKeyPath &right) const;

    /**Checks if two PropertyKeyPaths specify the same node. Hence all wildcards will be processed.\n
     * E.G.: "item1.child1.grandChild2" == "item1.*.grandChild2" is true.
     * \remark If you want to check if two paths are "truly" equal and not only equal in terms of
     * pointing to the same node, use the member function operator ==().*/
    bool Equals(const PropertyKeyPath &path) const;

    PropertyKeyPath &operator=(const PropertyKeyPath &path);

    /** Appends an "any element" to the path instance.*/
    PropertyKeyPath &AddAnyElement();
    /** Appends an element with the passed name to the path instance.*/
    PropertyKeyPath &AddElement(const ElementNameType &name);
    /** Appends an element with the passed name and any selection to the path instance.*/
    PropertyKeyPath &AddAnySelection(const ElementNameType &name);
    /** Appends an element with the passed name and selection index to the path instance.*/
    PropertyKeyPath &AddSelection(const ElementNameType &name, ItemSelectionIndex index);

    PropertyKeyPath();
    PropertyKeyPath(const PropertyKeyPath &path);

    ~PropertyKeyPath();

    void Reset();

  protected:
    NodeInfoVectorType m_NodeInfos;

    static bool PropertyKeyPathsMatch(const PropertyKeyPath &left, const PropertyKeyPath &right);
  };

  class MITKCORE_EXPORT InvalidPathNodeException : public mitk::Exception
  {
  public:
    mitkExceptionClassMacro(InvalidPathNodeException, mitk::Exception);
  };

  MITKCORE_EXPORT std::ostream &operator<<(std::ostream &os, const PropertyKeyPath &path);

  /**Helper function that converts a path PropertyKeyPath into a regex string that can be used
   to search for property keys (using std::regex) that are matched by the PropertyKeyPath.
  This function is used in context of the property persistence service.*/
  MITKCORE_EXPORT std::string PropertyKeyPathToPropertyRegEx(const PropertyKeyPath &tagPath);
  /**Helper function that converts a path PropertyKeyPath into a regex string that can be used
  to search for property persistence keys (using std::regex) that are matched by the PropertyKeyPath.
  This function is used in context of the property persistence service.*/
  MITKCORE_EXPORT std::string PropertyKeyPathToPersistenceKeyRegEx(const PropertyKeyPath &tagPath);
  /**Helper function that converts a path PropertyKeyPath into a regex that can be used as key template
  in a PropertyPersistanceInfo.
  This function is used in context of the property persistence service.*/
  MITKCORE_EXPORT std::string PropertyKeyPathToPersistenceKeyTemplate(const PropertyKeyPath &tagPath);
  /**Helper function that converts a path PropertyKeyPath into a regex that can be used as name template
  in a PropertyPersistanceInfo.
  This function is used in context of the property persistence service.*/
  MITKCORE_EXPORT std::string PropertyKeyPathToPersistenceNameTemplate(const PropertyKeyPath &tagPath);

  /** Converts the passed property name into a tag path. If the property name cannot be converted
   into a valid path, the returned path is empty.*/
  MITKCORE_EXPORT PropertyKeyPath PropertyNameToPropertyKeyPath(const std::string &propertyName);
  /** returns the correct property name for a given PropertyKeyPath instance. */
  MITKCORE_EXPORT std::string PropertyKeyPathToPropertyName(const PropertyKeyPath &tagPath);
} // namespace mitk

#endif
