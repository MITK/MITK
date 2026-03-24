/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentationTaskListMacros_h
#define mitkSegmentationTaskListMacros_h

#define mitkSegmentationTaskHasValueMacro(name) \
  bool Has##name() const { \
    return m_##name.has_value(); \
  }

#define mitkSegmentationTaskGetValueMacro(type, name) \
  type Get##name() const { \
    if (m_##name.has_value()) return m_##name.value(); \
    if (m_Defaults != nullptr && m_Defaults->m_##name.has_value()) return m_Defaults->m_##name.value(); \
    return type(); \
  }

#define mitkSegmentationTaskSetValueMacro(type, name) \
  void Set##name(const type& value) { \
    m_##name = value; \
  }

#define mitkSegmentationTaskValueMacro(type, name) \
  public: \
    mitkSegmentationTaskHasValueMacro(name) \
    mitkSegmentationTaskGetValueMacro(type, name) \
    mitkSegmentationTaskSetValueMacro(type, name) \
  private: \
    std::optional<type> m_##name;

#define mitkSegmentationTaskListGetValueMacro(type, name) \
  type Get##name(size_t index) const { \
    return index < m_Tasks.size() ? m_Tasks[index].Get##name() : type(); \
  }

#define mitkSegmentationTaskListHasValueMacro(name) \
  bool Has##name(size_t index) const { \
    return index < m_Tasks.size() && (m_Tasks[index].Has##name() || m_Defaults.Has##name()); \
  }

#define mitkSegmentationTaskListSetDefaultMacro(type, name) \
  void SetDefault##name(const type& value) { \
    m_Defaults.Set##name(value); \
  }

#define mitkSegmentationTaskListValueMacro(type, name) \
  mitkSegmentationTaskListHasValueMacro(name) \
  mitkSegmentationTaskListGetValueMacro(type, name) \
  mitkSegmentationTaskListSetDefaultMacro(type, name)

#endif
