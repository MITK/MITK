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

#ifndef mitkDICOMTagPath_h
#define mitkDICOMTagPath_h

#include <vector>

#include <mitkDICOMTag.h>

#include <MitkDICOMReaderExports.h>


namespace mitk
{
  /** @brief Class is used to identify (nested) attributes in a DICOM dataset.
  * In  contrast to the class DICOMTag, which only specifies one specific tag,
  * the tag path can identify nested attributes (like items in a DICOM sequence).
  * In addition you may also specify wildcards for the selection index or
  * complete elements of the path.
  * @remark If you want to keep the DICOMTagPath compatible to the dcmtk search path
  * format, you may *not* use element wild cards (this IsExplicit() or HasItemSelectionWildcardsOnly()
  * must return true).
  */
  class MITKDICOMREADER_EXPORT DICOMTagPath
  {
  public:
    typedef int ItemSelectionIndex;

    struct MITKDICOMREADER_EXPORT NodeInfo
    {
      enum class NodeType
      {
        Invalid = 0,  //*< Node is non existant or invalid.
        Element,  //*< Selects an specific element given the node name.
        SequenceSelection, //*< Selects an specific item in a sequence of items and has a item selector ("[n]").
        AnySelection, //*< Selects all items of a specific element ("[*]").
        AnyElement,  //*< Selects any element/item. Node name is wildcarded ("*"); item selection as well implictily.
      };

      NodeType type;
      DICOMTag tag;
      ItemSelectionIndex selection;

      NodeInfo();
      NodeInfo(const DICOMTag& tag, NodeType type = NodeType::Element, ItemSelectionIndex index = 0);
      bool Matches(const NodeInfo& right) const;

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

    const NodeInfoVectorType& GetNodes() const;

    std::string ToStr() const;

    DICOMTagPath& FromStr(const std::string& pathStr);

    /**Compares two DICOMTagPaths for real equality. So its a string compare of their string conversion*/
    bool operator == (const DICOMTagPath& path) const;

    /**Operation equals like comparing the ToStr() results with operator <.*/
    bool operator < (const DICOMTagPath& right) const;

    /**Checks if to DICOMTagPathes are specify the same node. Hence all wildcards will be processed.\n
    * E.G.: "item1/child1/grandChild2" == ".//item1//grandChild2" is true.
    * \remark If you want to check if to pathes are "truely" equal and not only equal in terms of
    * pointing to the same node, use the member function Equals()*/
    bool Equals(const DICOMTagPath& path) const;

    DICOMTagPath& operator = (const DICOMTagPath& path);

    DICOMTagPath& AddAnyElement();
    DICOMTagPath& AddElement(unsigned int group, unsigned int element);
    DICOMTagPath& AddAnySelection(unsigned int group, unsigned int element);
    DICOMTagPath& AddSelection(unsigned int group, unsigned int element, ItemSelectionIndex index);

    DICOMTagPath();
    DICOMTagPath(const DICOMTagPath& path);
    DICOMTagPath(const DICOMTag& tag);
    explicit DICOMTagPath(unsigned int group, unsigned int element);

    virtual ~DICOMTagPath();

    virtual void Reset();

  protected:
    NodeInfoVectorType m_NodeInfos;

    static bool DICOMTagPathesMatch(const DICOMTagPath& left, const DICOMTagPath& right);
  };

  typedef std::vector<DICOMTagPath> DICOMTagPathList;

  MITKDICOMREADER_EXPORT std::ostream& operator<<(std::ostream& os, const DICOMTagPath& path);

  MITKDICOMREADER_EXPORT std::string DICOMTagPathToPropertyRegEx(const DICOMTagPath& tagPath);
  MITKDICOMREADER_EXPORT std::string DICOMTagPathToPersistenceKeyRegEx(const DICOMTagPath& tagPath);
  MITKDICOMREADER_EXPORT std::string DICOMTagPathToPersistenceKeyTemplate(const DICOMTagPath& tagPath);
  MITKDICOMREADER_EXPORT std::string DICOMTagPathToPersistenceNameTemplate(const DICOMTagPath& tagPath);

  /** Converts a passed path into a search string for the DCMTK DcmPathProcessor.
   @pre tagPath must be an explicit (DICOMTagPath::IsExplicit()) path or
   must only contain selection wild cards (DICOMTagPath::HasItemSelectionWildcardsOnly()).*/
  MITKDICOMREADER_EXPORT std::string DICOMTagPathToDCMTKSearchPath(const DICOMTagPath& tagPath);

  /** Converts the passed property name into a tag path. If the property name cannot be converted
   into a valid path, the returned path is empty.*/
  MITKDICOMREADER_EXPORT DICOMTagPath PropertyNameToDICOMTagPath(const std::string& propertyName);
  /** returns the correct property name for a given DICOMTagPath instance. */
  MITKDICOMREADER_EXPORT std::string DICOMTagPathToPropertyName(const DICOMTagPath& tagPath);
}

#endif
