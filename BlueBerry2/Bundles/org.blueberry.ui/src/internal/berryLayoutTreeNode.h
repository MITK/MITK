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

#ifndef BERRYLAYOUTTREENODE_H_
#define BERRYLAYOUTTREENODE_H_

#include "berryLayoutTree.h"

#include "berryLayoutPartSash.h"

namespace berry {

/**
 * \ingroup org_blueberry_ui_internal
 *
 * Implementation of a tree node. The node represents a
 * sash and it allways has two children.
 */
class LayoutTreeNode : public LayoutTree {

public:

  osgiObjectMacro(LayoutTreeNode);

  struct ChildSizes {
    int left;
    int right;
    bool resizable;

  ChildSizes (int l, int r, bool resize);
  };

    /* The node children witch may be another node or a leaf */
    private: LayoutTree::Pointer children[2];

    /**
     * Initialize this tree with its sash.
     */
    public: LayoutTreeNode(LayoutPartSash::Pointer sash);

    public: ~LayoutTreeNode();

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.LayoutTree#flushChildren()
     */
    public: void FlushChildren();

    /**
     * Traverses the tree to find the part that intersects the given point
     *
     * @param toFind
     * @return the part that intersects the given point
     */
    public: LayoutPart::Pointer FindPart(const Point& toFind);

    /**
     * Add the relation ship between the children in the list
     * and returns the left children.
     */
    public: LayoutPart::Pointer ComputeRelation(std::list<PartSashContainer::RelationshipInfo>& relations);

    /**
     * Dispose all Sashs in this tree
     */
    public: void DisposeSashes();

    /**
     * Find a LayoutPart in the tree and return its sub-tree. Returns
     * null if the child is not found.
     */
    public: SmartPointer<LayoutTree> Find(LayoutPart::Pointer child);

    /**
     * Find the part that is in the bottom right position.
     */
    public: LayoutPart::Pointer FindBottomRight();

    /**
     * Go up in the tree finding a parent that is common of both children.
     * Return the subtree.
     */
    LayoutTreeNode* FindCommonParent(LayoutPart::Pointer child1, LayoutPart::Pointer child2,
            bool foundChild1 = false, bool foundChild2 = false);

    /**
     * Find a sash in the tree and return its sub-tree. Returns
     * null if the sash is not found.
     */
    public: SmartPointer<LayoutTreeNode> FindSash(LayoutPartSash::Pointer sash);

    /**
     * Sets the elements in the array of sashes with the
     * Left,Rigth,Top and Botton sashes. The elements
     * may be null depending whether there is a shash
     * beside the <code>part</code>
     */
    void FindSashes(SmartPointer<LayoutTree> child, PartPane::Sashes sashes);

    /**
     * Returns the sash of this node.
     */
    public: LayoutPartSash::Pointer GetSash() const;

    private: int GetSashSize() const;

    /**
     * Returns true if this tree has visible parts otherwise returns false.
     */
    public: bool IsVisible();

    /**
     * Remove the child and this node from the tree
     */
    SmartPointer<LayoutTree> Remove(SmartPointer<LayoutTree> child);

    /**
     * Replace a child with a new child and sets the new child's parent.
     */
    void ReplaceChild(SmartPointer<LayoutTree> oldChild, SmartPointer<LayoutTree> newChild);

    /**
     * Go up from the subtree and return true if all the sash are
     * in the direction specified by <code>isVertical</code>
     */
    public: bool SameDirection(bool isVertical, SmartPointer<LayoutTreeNode> subTree);

    public: int DoComputePreferredSize(bool width, int availableParallel, int availablePerpendicular, int preferredParallel);

    /**
     * Computes the pixel sizes of this node's children, given the available
     * space for this node. Note that "width" and "height" actually refer
     * to the distance perpendicular and parallel to the sash respectively.
     * That is, their meaning is reversed when computing a horizontal sash.
     *
     * @param width the pixel width of a vertical node, or the pixel height
     * of a horizontal node (INF if unbounded)
     * @param height the pixel height of a vertical node, or the pixel width
     * of a horizontal node (INF if unbounded)
     * @return a struct describing the pixel sizes of the left and right children
     * (this is a width for horizontal nodes and a height for vertical nodes)
     */
    ChildSizes ComputeChildSizes(int width, int height, int left, int right, int preferredWidth);

    protected: int DoGetSizeFlags(bool width);

    /**
     * Resize the parts on this tree to fit in <code>bounds</code>.
     */
    public: void DoSetBounds(const Rectangle& bounds);

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.LayoutTree#createControl(org.blueberry.swt.widgets.Composite)
     */
    public: void CreateControl(void* parent);

    //Added by hudsonr@us.ibm.com - bug 19524

    public: bool IsCompressible();

    /**
     * Returns 0 if there is no bias. Returns -1 if the first child should be of
     * fixed size, and the second child should be compressed. Returns 1 if the
     * second child should be of fixed size.
     * @return the bias
     */
    public: int GetCompressionBias();

    bool IsLeftChild(SmartPointer<const LayoutTree> toTest);

    SmartPointer<LayoutTree> GetChild(bool left);

    /**
     * Sets a child in this node
     */
    void SetChild(bool left, LayoutPart::Pointer part);

    /**
     * Sets a child in this node
     */
    void SetChild(bool left, SmartPointer<LayoutTree> child);

    /**
     * Returns a string representation of this object.
     */
    public: std::string ToString();

    /**
     * Create the sashes if the children are visible
     * and dispose it if they are not.
     */
    //public: void UpdateSashes(void* parent);

    /**
     * Writes a description of the layout to the given string buffer.
     * This is used for drag-drop test suites to determine if two layouts are the
     * same. Like a hash code, the description should compare as equal iff the
     * layouts are the same. However, it should be user-readable in order to
     * help debug failed tests. Although these are english readable strings,
     * they should not be translated or equality tests will fail.
     *
     * @param buf
     */
    public: void DescribeLayout(std::string& buf) const;

};

}

#endif /*BERRYLAYOUTTREENODE_H_*/
