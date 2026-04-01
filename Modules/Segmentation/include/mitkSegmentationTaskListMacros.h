/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentationTaskListMacros_h
#define mitkSegmentationTaskListMacros_h

/**
 * \file mitkSegmentationTaskListMacros.h
 * \brief Convenience macros for declaring SegmentationTask and SegmentationTaskList value properties.
 *
 * These macros generate Has/Get/Set methods backed by \c std::optional members. For task lists,
 * per-index accessors and default setters are generated as well.
 */

/** \brief Declare a Has<name>() method returning whether the optional value is set. */
#define mitkSegmentationTaskHasValueMacro(name) \
  bool Has##name() const { \
    return m_##name.has_value(); \
  }

/** \brief Declare a Get<name>() method returning the value or its default. */
#define mitkSegmentationTaskGetValueMacro(type, name) \
  type Get##name() const { \
    if (m_##name.has_value()) return m_##name.value(); \
    if (m_Defaults != nullptr && m_Defaults->m_##name.has_value()) return m_Defaults->m_##name.value(); \
    return type(); \
  }

/** \brief Declare a Set<name>() method that stores the value. */
#define mitkSegmentationTaskSetValueMacro(type, name) \
  void Set##name(const type& value) { \
    m_##name = value; \
  }

/** \brief Declare Has/Get/Set methods and the backing std::optional member for a task property. */
#define mitkSegmentationTaskValueMacro(type, name) \
  public: \
    mitkSegmentationTaskHasValueMacro(name) \
    mitkSegmentationTaskGetValueMacro(type, name) \
    mitkSegmentationTaskSetValueMacro(type, name) \
  private: \
    std::optional<type> m_##name;

/** \brief Declare a per-index Get<name>(index) method for the task list. */
#define mitkSegmentationTaskListGetValueMacro(type, name) \
  type Get##name(size_t index) const { \
    return index < m_Tasks.size() ? m_Tasks[index].Get##name() : type(); \
  }

/** \brief Declare a per-index Has<name>(index) method that checks the task or its defaults. */
#define mitkSegmentationTaskListHasValueMacro(name) \
  bool Has##name(size_t index) const { \
    return index < m_Tasks.size() && (m_Tasks[index].Has##name() || m_Defaults.Has##name()); \
  }

/** \brief Declare a SetDefault<name>() method for the task list default values. */
#define mitkSegmentationTaskListSetDefaultMacro(type, name) \
  void SetDefault##name(const type& value) { \
    m_Defaults.Set##name(value); \
  }

/** \brief Declare Has/Get/SetDefault methods for a task list property (combines list-level macros). */
#define mitkSegmentationTaskListValueMacro(type, name) \
  mitkSegmentationTaskListHasValueMacro(name) \
  mitkSegmentationTaskListGetValueMacro(type, name) \
  mitkSegmentationTaskListSetDefaultMacro(type, name)

#endif
