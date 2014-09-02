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


#ifndef _MITK_DOSE_ISO_LEVEL_COLLECTIONS_H_
#define _MITK_DOSE_ISO_LEVEL_COLLECTIONS_H_

#include <set>
#include <itkVectorContainer.h>

#include "mitkIsoDoseLevel.h"

namespace mitk
{
  /**
  \class IsoDoseLevelVector
  \brief Simple vector that stores dose iso levels.
  *
  * This class is for example used to store the user defined free iso values.
  */
  typedef ::itk::VectorContainer<unsigned int, mitk::IsoDoseLevel::Pointer> IsoDoseLevelVector;


  /**
  \class IsoDoseLevelSet
  \brief Stores values needed for the representation/visualization of dose iso levels.
  *
  * Set of dose iso levels sorted by the dose values of the iso levels (low to high values).
  * This data structure is used to represent the dose iso level setup used for the
  * visualization of a dose distribution.
  */
class MitkDicomRT_EXPORT IsoDoseLevelSet:
  public itk::Object
{
public:
  mitkClassMacro(IsoDoseLevelSet, itk::Object);
  itkNewMacro(Self);

private:
  /** Quick access to the STL vector type that was inherited. */
  typedef std::vector< IsoDoseLevel::Pointer > InternalVectorType;
  typedef InternalVectorType::size_type      size_type;
  typedef InternalVectorType::iterator       VectorIterator;
  typedef InternalVectorType::const_iterator VectorConstIterator;

  InternalVectorType m_IsoLevels;

protected:
  IsoDoseLevelSet() {};
  IsoDoseLevelSet(const IsoDoseLevelSet & other);

  virtual ~IsoDoseLevelSet() {};

  mitkCloneMacro(IsoDoseLevelSet);

public:
  typedef size_type IsoLevelIndexType;
  typedef IsoDoseLevel::DoseValueType DoseValueType;

  /** Convenient typedefs for the iterator and const iterator. */
  class ConstIterator;

  /** Friends to this class. */
  friend class ConstIterator;

  /** \class ConstIterator
   * Simulate STL-vector style const iteration where dereferencing the iterator
   * gives read access to the value.
   */
  class ConstIterator
  {
public:
    ConstIterator() {}
    ConstIterator(const VectorConstIterator & i): m_Iter(i) {}
    ConstIterator(const ConstIterator & r) { m_Iter = r.m_Iter; }
    const IsoDoseLevel & operator*()    { return *(m_Iter->GetPointer()); }
    const IsoDoseLevel * operator->()   { return m_Iter->GetPointer(); }
    ConstIterator & operator++()   {++m_Iter; return *this; }
    ConstIterator operator++(int) { ConstIterator temp(*this); ++m_Iter; return temp; }
    ConstIterator & operator--()   {--m_Iter; return *this; }
    ConstIterator operator--(int) { ConstIterator temp(*this); --m_Iter; return temp; }
    ConstIterator & operator=(const ConstIterator & r) {m_Iter = r.m_Iter; return *this; }
    bool operator==(const ConstIterator & r) const { return m_Iter == r.m_Iter; }
    bool operator!=(const ConstIterator & r) const { return m_Iter != r.m_Iter; }

    const IsoDoseLevel & Value(void) const { return *(m_Iter->GetPointer()); }

private:
    VectorConstIterator m_Iter;
  };

  /* Declare the public interface routines. */

  /**
   * Read the element from the given index.
   * It is assumed that the index exists.
   */
  const IsoDoseLevel& GetIsoDoseLevel(IsoLevelIndexType) const;
  const IsoDoseLevel& GetIsoDoseLevel(DoseValueType) const;

  /**
   * Set the element value at the given index.
   * It is assumed that the index exists.
   */
  void SetIsoDoseLevel(const IsoDoseLevel*);

  /**
   * Check if the index range of the vector is large enough to allow the
   * given index without expansion.
   */
  bool DoseLevelExists(IsoLevelIndexType) const;
  bool DoseLevelExists(DoseValueType) const;

  /**
   * Delete the element defined by the index identifier.  In practice, it
   * doesn't make sense to delete a vector index.  Instead, this method just
   * overwrites the index with the default element.
   */
  void DeleteIsoDoseLevel(DoseValueType);
  void DeleteIsoDoseLevel(IsoLevelIndexType);

  /**
   * Get a begin const iterator for the vector.
   */
  ConstIterator Begin(void) const;

  /**
   * Get an end const iterator for the vector.
   */
  ConstIterator End(void) const;

  /**
   * Get the number of elements currently stored in the vector.
   */
  IsoLevelIndexType Size(void) const;

  /**
   * Clear the elements. The final size will be zero.
   */
  void Reset(void);
};

}

#endif //_MITK_DOSE_ISO_LEVEL_COLLECTIONS_H_
