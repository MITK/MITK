/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkTreeContainerBase_h
#define __itkTreeContainerBase_h

#include "itkMacro.h"
#include <itkObject.h>
#include <iostream>
#include <itkObjectFactory.h>
#include <itkTreeNode.h>

//#include <itkTreeIteratorBase.h>

namespace itk
{

//template <class TValueType > class TreeIteratorBase;

/** \class TreeContainerBase
 *  \brief Array class with size defined at construction time.
 * 
 * This class derives from the vnl_vector<> class. 
 * Its size is assigned at construction time (run time) and can 
 * not be changed afterwards except by using assignment to another
 * Array.
 *
 * The class is templated over the type of the elements.
 *
 * Template parameters for class Array:
 *
 * - TValueType = Element type stored at each location in the array.
 *
 * \ingroup DataRepresentation 
 */
template <class TValueType>
class TreeContainerBase : public Object
{
 
public:

  typedef Object Superclass;
  typedef TreeContainerBase Self;
  typedef SmartPointer<Self>  Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  typedef TValueType ValueType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(TreeContainerBase, Object);

  //typedef TreeIteratorBase<Self> IteratorType;
 // typedef int IteratorType;

    virtual bool SetRoot( const TValueType element ) = 0;

    virtual bool Contains( const TValueType element ) = 0;

    virtual int Count() const = 0;

 //   virtual bool operator==( TreeContainerBase<TValueType>& tree ) = 0;

  virtual bool IsLeaf( const TValueType element ) = 0;

  virtual bool IsRoot( const TValueType element ) = 0;

  virtual bool Clear( ) = 0;

//  virtual bool SetRoot( IteratorType& pos ) = 0;
  
  virtual bool SetRoot( TreeNode<TValueType>* node) = 0;

//  virtual bool Swap( IteratorType& v, IteratorType& w ) = 0;

  virtual const TreeNode<TValueType>* GetRoot() const = 0;
  
  /** Set if the tree is a subtree */
  void SetSubtree(bool val) {m_SubTree = val;}

protected:

  TreeContainerBase(){}; 
  virtual ~TreeContainerBase(){}; 
  bool  m_SubTree;

};



} // namespace itk



#endif
