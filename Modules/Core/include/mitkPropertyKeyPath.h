/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyKeyPath_h
#define mitkPropertyKeyPath_h

#include <string>
#include <vector>
#include <map>

#include <mitkExceptionMacro.h>

#include <MitkCoreExports.h>

namespace mitk
{
  class IPropertyProvider;

  /** @brief Class that can be used to specify nested or wild carded property keys. E.g.
   * for the use in context of the property persistence service or the property relation service.\n
   * Following assumptions are made /preconditions are defined:
   * - A property key is partitioned by "." into nodes (c.f. visualization of property keys in the PropertyView).
   * - A node can either be an element or a selection.
   * - An element has a name (alphanumric, - and space; "A-Za-z0-9- ") or is wildcarded ("*")
   * - A selection is either an index (e.g. "[1]") or a wildcard ("[*]").
   *
   * Selections are used to indicate that the preceding element has multiple occurrences and which occurrence is meant.
   * Example property keys would be:
   * - prop : A simple property key
   * - prop.subprop1 : A property key consisting of two nodes
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

    /**
     * \brief Information about a single node in a PropertyKeyPath.
     *
     * Each node has a type, an optional name, and an optional selection index.
     */
    struct MITKCORE_EXPORT NodeInfo
    {
      /** \brief Types of nodes within a PropertyKeyPath. */
      enum class NodeType
      {
        Invalid = 0,      /**< Node does not exist or is invalid. */
        Element,          /**< Selects a specific element given the node name. */
        ElementSelection, /**< Selects a specific item in a sequence by index ("[n]"). */
        AnySelection,     /**< Selects all items of a specific element ("[*]"). */
        AnyElement        /**< Selects any element/item. Node name is wildcarded ("*"); item selection as well implicitly. */
      };

      /** \brief The type of this node. */
      NodeType type;

      /** \brief The element name for this node (empty for AnyElement). */
      ElementNameType name;

      /** \brief The selection index for ElementSelection nodes. */
      ItemSelectionIndex selection;

      /** \brief Default constructor. Creates an Invalid node. */
      NodeInfo();

      /**
       * \brief Construct a node with given name, type, and selection index.
       *
       * \param[in] name The element name.
       * \param[in] type The node type (default: Element).
       * \param[in] index The selection index (default: 0).
       */
      NodeInfo(const ElementNameType &name, NodeType type = NodeType::Element, ItemSelectionIndex index = 0);

      /**
       * \brief Check if this node matches another, respecting wildcards.
       *
       * AnyElement matches any other valid node. AnySelection matches any
       * selection index of the same element name.
       *
       * \param[in] right The node to match against.
       * \return \c true if the nodes match.
       */
      bool Matches(const NodeInfo &right) const;

      /**
       * \brief Check for exact equality (same type, name, and selection).
       *
       * \param[in] right The node to compare against.
       * \return \c true if both nodes are identical.
       */
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

    /**
     * \brief Get a const reference to all node info entries.
     * \return The vector of NodeInfo entries.
     */
    const NodeInfoVectorType &GetNodes() const;

    /**
     * \brief Compare two PropertyKeyPaths for exact structural equality.
     *
     * Two paths are equal if and only if all their nodes are identical
     * (same type, name, and selection index).
     *
     * \param[in] path The path to compare against.
     * \return \c true if both paths are structurally identical.
     *
     * \sa Equals
     */
    bool operator==(const PropertyKeyPath &path) const;

    /**
     * \brief Lexicographic less-than comparison.
     *
     * Compares node names, types, and selections element by element.
     *
     * \param[in] right The path to compare against.
     * \return \c true if this path is lexicographically less than \p right.
     */
    bool operator<(const PropertyKeyPath &right) const;

    /**
     * \brief Lexicographic less-than-or-equal comparison.
     *
     * \param[in] right The path to compare against.
     * \return \c true if this path is lexicographically less than or equal to \p right.
     */
    bool operator<=(const PropertyKeyPath &right) const;

    /**
     * \brief Lexicographic greater-than-or-equal comparison.
     *
     * \param[in] right The path to compare against.
     * \return \c true if this path is lexicographically greater than or equal to \p right.
     */
    bool operator>=(const PropertyKeyPath &right) const;

    /**
     * \brief Lexicographic greater-than comparison.
     *
     * \param[in] right The path to compare against.
     * \return \c true if this path is lexicographically greater than \p right.
     */
    bool operator>(const PropertyKeyPath &right) const;

    /**
     * \brief Check if two PropertyKeyPaths match, processing wildcards.
     *
     * Unlike operator==(), this method treats wildcards as matching any
     * corresponding node. For example, "item1.child1.grandChild2" matches
     * "item1.*.grandChild2".
     *
     * \param[in] path The path to compare against.
     * \return \c true if the paths match (considering wildcards).
     *
     * \note Use operator==() to check for exact structural equality.
     */
    bool Equals(const PropertyKeyPath &path) const;

    /**
     * \brief Assignment operator.
     *
     * \param[in] path The path to copy from.
     * \return A reference to this path.
     */
    PropertyKeyPath &operator=(const PropertyKeyPath &path);

    /**
     * \brief Append a wildcard "any element" node to the path.
     *
     * The resulting node matches any element name and selection.
     *
     * \return A reference to this path (for chaining).
     */
    PropertyKeyPath &AddAnyElement();

    /**
     * \brief Append a named element node to the path.
     *
     * \param[in] name The element name.
     * \return A reference to this path (for chaining).
     */
    PropertyKeyPath &AddElement(const ElementNameType &name);

    /**
     * \brief Append a named element with wildcard selection to the path.
     *
     * The resulting node matches any selection index for the given element name.
     *
     * \param[in] name The element name.
     * \return A reference to this path (for chaining).
     */
    PropertyKeyPath &AddAnySelection(const ElementNameType &name);

    /**
     * \brief Append a named element with a specific selection index to the path.
     *
     * \param[in] name The element name.
     * \param[in] index The selection index.
     * \return A reference to this path (for chaining).
     */
    PropertyKeyPath &AddSelection(const ElementNameType &name, ItemSelectionIndex index);

    /** \brief Default constructor. Creates an empty path. */
    PropertyKeyPath();

    /**
     * \brief Copy constructor.
     * \param[in] path The path to copy.
     */
    PropertyKeyPath(const PropertyKeyPath &path);

    /**
     * \brief Construct a path from a list of element names.
     *
     * Creates a simple key path consisting only of Element nodes.
     *
     * \param[in] list An initializer list of element name strings.
     */
    PropertyKeyPath(const std::initializer_list< ElementNameType >& list);

    ~PropertyKeyPath();

    /**
     * \brief Clear the path, removing all nodes.
     */
    void Reset();

  protected:
    NodeInfoVectorType m_NodeInfos;

    static bool PropertyKeyPathsMatch(const PropertyKeyPath &left, const PropertyKeyPath &right);
  };

  /**
   * \brief Exception thrown when an invalid path node index is accessed.
   *
   * \sa PropertyKeyPath::GetNode
   * \sa PropertyKeyPath::GetFirstNode
   * \sa PropertyKeyPath::GetLastNode
   */
  class MITKCORE_EXPORT InvalidPathNodeException : public mitk::Exception
  {
  public:
    mitkExceptionClassMacro(InvalidPathNodeException, mitk::Exception);
  };

  /**
   * \brief Stream output operator for PropertyKeyPath.
   *
   * Writes the property name representation to the stream.
   *
   * \param[in,out] os The output stream.
   * \param[in] path The PropertyKeyPath to output.
   * \return The output stream.
   */
  MITKCORE_EXPORT std::ostream &operator<<(std::ostream &os, const PropertyKeyPath &path);

  /**
   * \brief Convert a PropertyKeyPath to a regex string for matching property keys.
   *
   * Wildcards in the path are converted to regex capture groups. Used by the
   * property persistence service.
   *
   * \param[in] tagPath The PropertyKeyPath to convert.
   * \return A regex string suitable for std::regex matching of property keys.
   */
  MITKCORE_EXPORT std::string PropertyKeyPathToPropertyRegEx(const PropertyKeyPath &tagPath);

  /**
   * \brief Convert a PropertyKeyPath to a regex string for matching persistence keys.
   *
   * Similar to PropertyKeyPathToPropertyRegEx() but uses underscores instead of dots
   * as separators. Used by the property persistence service.
   *
   * \param[in] tagPath The PropertyKeyPath to convert.
   * \return A regex string for matching persistence keys.
   */
  MITKCORE_EXPORT std::string PropertyKeyPathToPersistenceKeyRegEx(const PropertyKeyPath &tagPath);

  /**
   * \brief Convert a PropertyKeyPath to a persistence key template string.
   *
   * Wildcards are replaced by regex back-references ($1, $2, ...) for use
   * as a key template in PropertyPersistenceInfo. Uses underscores as separators.
   *
   * \param[in] tagPath The PropertyKeyPath to convert.
   * \return A key template string.
   */
  MITKCORE_EXPORT std::string PropertyKeyPathToPersistenceKeyTemplate(const PropertyKeyPath &tagPath);

  /**
   * \brief Convert a PropertyKeyPath to a persistence name template string.
   *
   * Wildcards are replaced by regex back-references ($1, $2, ...) for use
   * as a name template in PropertyPersistenceInfo. Uses dots as separators.
   *
   * \param[in] tagPath The PropertyKeyPath to convert.
   * \return A name template string.
   */
  MITKCORE_EXPORT std::string PropertyKeyPathToPersistenceNameTemplate(const PropertyKeyPath &tagPath);

  /**
   * \brief Convert a property name string to a PropertyKeyPath.
   *
   * Parses the dot-separated property name and creates the corresponding
   * path structure. If the name cannot be parsed, an empty path is returned.
   *
   * \param[in] propertyName The property name string to convert.
   * \return The corresponding PropertyKeyPath, or an empty path if parsing fails.
   */
  MITKCORE_EXPORT PropertyKeyPath PropertyNameToPropertyKeyPath(const std::string &propertyName);

  /**
   * \brief Convert a PropertyKeyPath to its property name string representation.
   *
   * Creates a dot-separated property name from the path nodes, using "[n]" for
   * element selections and "[*]" for any selections.
   *
   * \param[in] tagPath The PropertyKeyPath to convert.
   * \return The property name string.
   */
  MITKCORE_EXPORT std::string PropertyKeyPathToPropertyName(const PropertyKeyPath &tagPath);

  /**
   * \brief Find indexed entries matching a PropertyKeyPath in a property provider.
   *
   * Scans all property keys of the given provider and finds those matching the
   * path pattern. The path must contain exactly one AnySelection node; the
   * matched selection indices and their full property names are returned.
   *
   * \param[in] provider Pointer to the property provider whose keys are scanned.
   * \param[in] path PropertyKeyPath with exactly one AnySelection node.
   * \return A map with the selection index as key and the full property name as value.
   *
   * \pre \p path must contain exactly one AnySelection node.
   * \throw mitk::Exception if the path does not contain exactly one AnySelection node.
   */
  MITKCORE_EXPORT std::map<PropertyKeyPath::ItemSelectionIndex, std::string>
    FindIndexedPropertyNames(const IPropertyProvider* provider, const PropertyKeyPath& path);

} // namespace mitk

#endif
