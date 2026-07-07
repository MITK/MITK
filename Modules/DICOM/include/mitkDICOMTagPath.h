/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMTagPath_h
#define mitkDICOMTagPath_h

#include <vector>

#include <mitkDICOMTag.h>

#include <MitkDICOMExports.h>


namespace mitk
{
  /**
   * \ingroup DICOMModule
   * \brief Class used to identify (nested) attributes in a DICOM dataset.
   *
   * In contrast to the class DICOMTag, which only specifies one specific tag,
   * the tag path can identify nested attributes (like items in a DICOM sequence).
   * In addition you may also specify wildcards for the selection index or
   * complete elements of the path.
   *
   * \note If you want to keep the DICOMTagPath compatible to the DCMTK search path
   * format, you may *not* use element wild cards (i.e. IsExplicit() or HasItemSelectionWildcardsOnly()
   * must return true).
   *
   * \sa DICOMTag, DICOMTagPathToPropertyName, PropertyNameToDICOMTagPath
   */
  class MITKDICOM_EXPORT DICOMTagPath
  {
  public:
    typedef int ItemSelectionIndex;

    /** \brief Describes a single node within a DICOMTagPath. */
    struct MITKDICOM_EXPORT NodeInfo
    {
      /** \brief Defines the type of a path node. */
      enum class NodeType
      {
        Invalid = 0,       ///< Node is non-existent or invalid.
        Element,           ///< Selects a specific element given the node name.
        SequenceSelection, ///< Selects a specific item in a sequence of items and has an item selector ("[n]").
        AnySelection,      ///< Selects all items of a specific element ("[*]").
        AnyElement,        ///< Selects any element/item. Node name is wildcarded ("*"); item selection as well implicitly.
      };

      NodeType type;               ///< The type of this node.
      DICOMTag tag;                ///< The DICOM tag this node refers to.
      ItemSelectionIndex selection; ///< The item selection index (for SequenceSelection nodes).

      /** \brief Default constructor, creates an Invalid node. */
      NodeInfo();

      /**
       * \brief Construct a node info with specified tag, type, and optional selection index.
       * \param[in] tag The DICOM tag for this node.
       * \param[in] type The node type (default: Element).
       * \param[in] index The item selection index (default: 0).
       */
      NodeInfo(const DICOMTag& tag, NodeType type = NodeType::Element, ItemSelectionIndex index = 0);

      /**
       * \brief Check whether this node matches another node (respecting wildcards).
       * \param[in] right The node to match against.
       * \return true if this node matches the given node.
       */
      bool Matches(const NodeInfo& right) const;

      /**
       * \brief Equality operator; checks strict equality (no wildcard matching).
       * \param[in] right The node to compare with.
       * \return true if type, tag, and selection are identical.
       */
      bool operator == (const NodeInfo& right) const;
    };

    typedef std::vector<NodeInfo> NodeInfoVectorType;
    typedef NodeInfoVectorType::size_type PathIndexType;

    /** Returns if the DICOMTagPath is empty.*/
    bool IsEmpty() const;

    /** Returns if the path is explicit (has no wildcards).*/
    bool IsExplicit() const;

    /** Returns if the path has any nodes with item selection wild cards ([*]).*/
    bool HasItemSelectionWildcardsOnly() const;

    /** Number of path nodes the DICOMTagPath contains.*/
    PathIndexType Size() const;

    /** Adds a new node to the end of the path.
    \param [in] newNode Reference to the node that should be added.
    \return Returns the index of the newly added node.*/
    PathIndexType AddNode(const NodeInfo& newNode);

    /** Function returns the node info of a path node specified by the index
    * within the DICOMTagPath.
    * \pre Passed index must not be out of bound.
    * \param [in] index Index of the node whose info should be retrieved.
    * \return Info of the specified path node. If the index is out of bound an InvalidPathNode exception will be thrown.*/
    const NodeInfo& GetNode(const PathIndexType& index) const;

    /** Function returns the node info of a path node specified by the index
    * within the DICOMTagPath.
    * \pre Passed index must not be out of bound.
    * \param [in] index Index of the node whose info should be retrieved.
    * \return Info of the specified path node. If the index is out of bound an InvalidPathNode exception will be thrown.*/
    NodeInfo& GetNode(const PathIndexType& index);

    /** Function returns the node info of the first path node within the DICOMTagPath.
    * \pre DICOMTagPath must not be empty.
    * \return Info of the first path node. If the path is empty, an InvalidPathNode exception will be thrown.*/
    NodeInfo& GetFirstNode();

    /** Function returns the node info of the first path node within the DICOMTagPath.
    * \pre DICOMTagPath must not be empty.
    * \return Info of the first path node. If the path is empty, an InvalidPathNode exception will be thrown.*/
    const NodeInfo& GetFirstNode() const;

    /** Function returns the node info of the last path node within the DICOMTagPath.
    * \pre DICOMTagPath must not be empty.
    * \return Info of the first path node. If the path is empty, an InvalidPathNode exception will be thrown.*/
    NodeInfo& GetLastNode();

    /** Function returns the node info of the last path node within the DICOMTagPath.
    * \pre DICOMTagPath must not be empty.
    * \return Info of the first path node. If the path is empty, an InvalidPathNode exception will be thrown.*/
    const NodeInfo& GetLastNode() const;

    /**
     * \brief Get the complete vector of node infos in this path.
     * \return Const reference to the internal node info vector.
     */
    const NodeInfoVectorType& GetNodes() const;

    /**
     * \brief Serialize this path to a string representation.
     * \return A string representation of this DICOMTagPath.
     * \sa FromStr
     */
    std::string ToStr() const;

    /**
     * \brief Parse a string representation and set this path accordingly.
     * \param[in] pathStr The string to parse.
     * \return Reference to this path after parsing.
     * \sa ToStr
     */
    DICOMTagPath& FromStr(const std::string& pathStr);

    /**Compares two DICOMTagPaths for real equality. So its a string compare of their string conversion*/
    bool operator == (const DICOMTagPath& path) const;

    /**Operation equals like comparing the ToStr() results with operator <.*/
    bool operator < (const DICOMTagPath& right) const;

    /**Checks if to DICOMTagPathes are specify the same node. Hence all wildcards will be processed.\n
    * E.G.: "item1/child1/grandChild2" == ".//item1//grandChild2" is true.
    * \remark If you want to check if two paths are "truly" equal and not only equal in terms of
    * pointing to the same node, use the member function Equals()*/
    bool Equals(const DICOMTagPath& path) const;

    DICOMTagPath& operator = (const DICOMTagPath& path);

    /** Concatenates two DICOM tag paths.
    * \param [in] right The path to append to this path.
    * \return A new DICOMTagPath containing all nodes from both paths.*/
    DICOMTagPath operator + (const DICOMTagPath& right) const;

    /** Concatenates two DICOM tag paths.
    * \param [in] pathStr The path to append to this path.
    * \return A new DICOMTagPath containing all nodes from both paths.*/
    DICOMTagPath operator + (const std::string& pathStr) const;

    /** Appends another path to this path.
    * \param [in] right The path to append.
    * \return Reference to this path.*/
    DICOMTagPath& operator += (const DICOMTagPath& right);

    /** Appends a string representation of a path to this path.
    * \param [in] pathStr String representation of a path to append.
    * \return Reference to this path.*/
    DICOMTagPath& operator += (const std::string& pathStr);

    /**
     * \brief Append a wildcard element node (AnyElement) to this path.
     * \return Reference to this path.
     */
    DICOMTagPath& AddAnyElement();

    /**
     * \brief Append a specific element node to this path.
     * \param[in] group The DICOM tag group number.
     * \param[in] element The DICOM tag element number.
     * \return Reference to this path.
     */
    DICOMTagPath& AddElement(unsigned int group, unsigned int element);

    /**
     * \brief Append a wildcard selection node (AnySelection) to this path.
     * \param[in] group The DICOM tag group number.
     * \param[in] element The DICOM tag element number.
     * \return Reference to this path.
     */
    DICOMTagPath& AddAnySelection(unsigned int group, unsigned int element);

    /**
     * \brief Append a specific sequence selection node to this path.
     * \param[in] group The DICOM tag group number.
     * \param[in] element The DICOM tag element number.
     * \param[in] index The item selection index within the sequence.
     * \return Reference to this path.
     */
    DICOMTagPath& AddSelection(unsigned int group, unsigned int element, ItemSelectionIndex index);

    /** \brief Default constructor, creates an empty path. */
    DICOMTagPath();

    /** \brief Copy constructor. */
    DICOMTagPath(const DICOMTagPath& path);

    /**
     * \brief Construct a single-element path from a DICOMTag.
     * \param[in] tag The DICOM tag to use as the single path node.
     */
    DICOMTagPath(const DICOMTag& tag);

    /**
     * \brief Construct a single-element path from group and element numbers.
     * \param[in] group The DICOM tag group number.
     * \param[in] element The DICOM tag element number.
     */
    explicit DICOMTagPath(unsigned int group, unsigned int element);

    virtual ~DICOMTagPath();

    /** \brief Clear all nodes from this path, making it empty. */
    virtual void Reset();

  protected:
    NodeInfoVectorType m_NodeInfos;

    static bool DICOMTagPathesMatch(const DICOMTagPath& left, const DICOMTagPath& right);
  };

  /** \brief A list of DICOMTagPath instances. */
  typedef std::vector<DICOMTagPath> DICOMTagPathList;

  /** \brief Stream output operator for DICOMTagPath. */
  MITKDICOM_EXPORT std::ostream& operator<<(std::ostream& os, const DICOMTagPath& path);

  /** \brief Concatenates a string with a DICOM tag path (reverse order).
  * \param[in] pathStr String representation of a path.
  * \param[in] right The DICOMTagPath to append.
  * \return A new DICOMTagPath with the concatenated result.*/
  MITKDICOM_EXPORT DICOMTagPath operator + (const std::string& pathStr, const DICOMTagPath& right);

  /**
   * \brief Convert a DICOMTagPath to a regular expression that matches the corresponding property name.
   * \param[in] tagPath The path to convert.
   * \return A regular expression string.
   */
  MITKDICOM_EXPORT std::string DICOMTagPathToPropertyRegEx(const DICOMTagPath& tagPath);

  /**
   * \brief Convert a DICOMTagPath to a regular expression for persistence key matching.
   * \param[in] tagPath The path to convert.
   * \return A regular expression string for persistence keys.
   */
  MITKDICOM_EXPORT std::string DICOMTagPathToPersistenceKeyRegEx(const DICOMTagPath& tagPath);

  /**
   * \brief Convert a DICOMTagPath to a persistence key template string.
   * \param[in] tagPath The path to convert.
   * \return A persistence key template string.
   */
  MITKDICOM_EXPORT std::string DICOMTagPathToPersistenceKeyTemplate(const DICOMTagPath& tagPath);

  /**
   * \brief Convert a DICOMTagPath to a persistence name template string.
   * \param[in] tagPath The path to convert.
   * \return A persistence name template string.
   */
  MITKDICOM_EXPORT std::string DICOMTagPathToPersistenceNameTemplate(const DICOMTagPath& tagPath);

  /** Converts a passed path into a search string for the DCMTK DcmPathProcessor.
   \pre tagPath must be an explicit (DICOMTagPath::IsExplicit()) path or
   must only contain selection wild cards (DICOMTagPath::HasItemSelectionWildcardsOnly()).*/
  MITKDICOM_EXPORT std::string DICOMTagPathToDCMTKSearchPath(const DICOMTagPath& tagPath);

  /** Converts the passed property name into a tag path. If the property name cannot be converted
   into a valid path, the returned path is empty.*/
  MITKDICOM_EXPORT DICOMTagPath PropertyNameToDICOMTagPath(const std::string& propertyName);

  /** returns the correct property name for a given DICOMTagPath instance. */
  MITKDICOM_EXPORT std::string DICOMTagPathToPropertyName(const DICOMTagPath& tagPath);

  /** Converts a DICOM tag path to a human-readable description.
   * This function uses DCMTK/GDCM dictionaries to resolve tag names.
   * For sequences, it shows the hierarchy with proper descriptions.
   * \param [in] tagPath The path to convert.
   * \param [in] includeTagNumbers If true, includes tag numbers in format "Name (GGGG,EEEE)".
   * \return A human-readable string describing the path.
   * Example: "Patient Name" or "Referenced Study Sequence[0].Study Instance UID"
   */
  MITKDICOM_EXPORT std::string DICOMTagPathToReadableName(
    const DICOMTagPath& tagPath,
    bool includeTagNumbers = false);
}

#endif
