/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkIsoDoseLevelCollections_h
#define mitkIsoDoseLevelCollections_h

#include <set>
#include <itkVectorContainer.h>

#include <mitkIsoDoseLevel.h>

namespace mitk
{
  /**
   * \class IsoDoseLevelVector
   * \brief Simple vector container that stores dose iso levels.
   *
   * This class is used to store user-defined free iso values as an ordered
   * collection of IsoDoseLevel smart pointers.
   *
   * \sa IsoDoseLevel
   * \sa IsoDoseLevelSet
   * \sa IsoDoseLevelVectorProperty
   */
  typedef ::itk::VectorContainer<unsigned int, mitk::IsoDoseLevel::Pointer> IsoDoseLevelVector;


  /**
   * \class IsoDoseLevelSet
   * \brief Sorted set of dose iso levels for dose distribution visualization.
   *
   * Maintains a collection of IsoDoseLevel instances sorted by their dose values
   * (from low to high). This data structure is the primary means for defining
   * the color coding and visibility rules used to visualize a dose distribution.
   * Duplicate dose values are not allowed; inserting a level with an existing
   * dose value replaces the previous entry.
   *
   * \sa IsoDoseLevel
   * \sa IsoDoseLevelVector
   * \sa IsoDoseLevelSetProperty
   */
class MITKRT_EXPORT IsoDoseLevelSet:
  public itk::Object
{
public:
  mitkClassMacroItkParent(IsoDoseLevelSet, itk::Object);
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
  explicit IsoDoseLevelSet(const IsoDoseLevelSet & other);

  ~IsoDoseLevelSet() override {};

  mitkCloneMacro(IsoDoseLevelSet);

public:
  /** \brief Index type for accessing iso levels by position. */
  typedef size_type IsoLevelIndexType;

  /** \brief Dose value type inherited from IsoDoseLevel. */
  typedef IsoDoseLevel::DoseValueType DoseValueType;

  /** Convenient typedefs for the iterator and const iterator. */
  class ConstIterator;

  /** Friends to this class. */
  friend class ConstIterator;

  /**
   * \class ConstIterator
   * \brief STL-style const iterator for read-only traversal of the iso dose level set.
   *
   * Dereferencing the iterator provides read access to the underlying IsoDoseLevel.
   */
  class ConstIterator
  {
public:
    /** \brief Default constructor. */
    ConstIterator() {}

    /**
     * \brief Constructs a ConstIterator from an internal vector const iterator.
     * \param[in] i The internal const iterator to wrap.
     */
    ConstIterator(const VectorConstIterator & i): m_Iter(i) {}

    /**
     * \brief Copy constructor.
     * \param[in] r The ConstIterator to copy.
     */
    ConstIterator(const ConstIterator & r) { m_Iter = r.m_Iter; }

    /** \brief Dereference operator. \return Reference to the current IsoDoseLevel. */
    const IsoDoseLevel & operator*()    { return *(m_Iter->GetPointer()); }

    /** \brief Arrow operator. \return Pointer to the current IsoDoseLevel. */
    const IsoDoseLevel * operator->()   { return m_Iter->GetPointer(); }

    /** \brief Pre-increment operator. \return Reference to the advanced iterator. */
    ConstIterator & operator++()   {++m_Iter; return *this; }

    /** \brief Post-increment operator. \return Iterator at the previous position. */
    ConstIterator operator++(int) { ConstIterator temp(*this); ++m_Iter; return temp; }

    /** \brief Pre-decrement operator. \return Reference to the retreated iterator. */
    ConstIterator & operator--()   {--m_Iter; return *this; }

    /** \brief Post-decrement operator. \return Iterator at the previous position. */
    ConstIterator operator--(int) { ConstIterator temp(*this); --m_Iter; return temp; }

    /** \brief Assignment operator. \param[in] r Iterator to copy. \return Reference to this. */
    ConstIterator & operator=(const ConstIterator & r) {m_Iter = r.m_Iter; return *this; }

    /** \brief Equality comparison. \param[in] r Iterator to compare. \return True if iterators are equal. */
    bool operator==(const ConstIterator & r) const { return m_Iter == r.m_Iter; }

    /** \brief Inequality comparison. \param[in] r Iterator to compare. \return True if iterators differ. */
    bool operator!=(const ConstIterator & r) const { return m_Iter != r.m_Iter; }

    /** \brief Get the value at the current position. \return Const reference to the IsoDoseLevel. */
    const IsoDoseLevel & Value(void) const { return *(m_Iter->GetPointer()); }

private:
    VectorConstIterator m_Iter;
  };

  /* Declare the public interface routines. */

  /**
   * \brief Retrieve an iso dose level by its positional index.
   * \param[in] index Zero-based index of the level.
   * \return Const reference to the IsoDoseLevel at the given index.
   * \throw mitk::Exception if the index is out of range.
   */
  const IsoDoseLevel& GetIsoDoseLevel(IsoLevelIndexType index) const;

  /**
   * \brief Retrieve an iso dose level by its dose value.
   * \param[in] value The relative dose value to search for.
   * \return Const reference to the matching IsoDoseLevel.
   * \throw mitk::Exception if no level with the given dose value exists.
   */
  const IsoDoseLevel& GetIsoDoseLevel(DoseValueType value) const;

  /**
   * \brief Insert or replace an iso dose level.
   *
   * If a level with the same dose value already exists, it is removed first.
   * The new level is cloned and inserted, and the set is re-sorted.
   *
   * \param[in] level Pointer to the IsoDoseLevel to insert. Must not be nullptr.
   * \throw mitk::Exception if level is nullptr.
   */
  void SetIsoDoseLevel(const IsoDoseLevel* level);

  /**
   * \brief Check whether an iso dose level exists at the given index.
   * \param[in] index The positional index to check.
   * \return True if the index is within the valid range.
   */
  bool DoseLevelExists(IsoLevelIndexType index) const;

  /**
   * \brief Check whether an iso dose level with the given dose value exists.
   * \param[in] value The relative dose value to search for.
   * \return True if a level with that dose value is found.
   */
  bool DoseLevelExists(DoseValueType value) const;

  /**
   * \brief Remove an iso dose level identified by its dose value.
   * \param[in] value The relative dose value of the level to remove. No-op if not found.
   */
  void DeleteIsoDoseLevel(DoseValueType value);

  /**
   * \brief Remove an iso dose level identified by its positional index.
   * \param[in] index The zero-based index of the level to remove. No-op if out of range.
   */
  void DeleteIsoDoseLevel(IsoLevelIndexType index);

  /**
   * \brief Get a const iterator pointing to the first iso dose level.
   * \return A ConstIterator at the beginning of the set.
   */
  ConstIterator Begin(void) const;

  /**
   * \brief Get a const iterator pointing past the last iso dose level.
   * \return A ConstIterator at the end of the set.
   */
  ConstIterator End(void) const;

  /**
   * \brief Get the number of iso dose levels in the set.
   * \return The number of elements currently stored.
   */
  IsoLevelIndexType Size(void) const;

  /**
   * \brief Remove all iso dose levels from the set.
   * \post Size() == 0.
   */
  void Reset(void);
};

}

#endif
