/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentationTaskMacros_h
#define mitkSegmentationTaskMacros_h

#define mitkSegmentationSubtaskHasValueMacro(name) \
  bool Has##name() const { \
    return m_##name.has_value(); \
  }

#define mitkSegmentationSubtaskGetValueMacro(type, name) \
  type Get##name() const { \
    if (m_##name.has_value()) return m_##name.value(); \
    if (m_Defaults != nullptr && m_Defaults->m_##name.has_value()) return m_Defaults->m_##name.value(); \
    return type(); \
  }

#define mitkSegmentationSubtaskSetValueMacro(type, name) \
  void Set##name(const type& value) { \
    m_##name = value; \
  }

#define mitkSegmentationSubtaskValueMacro(type, name) \
  public: \
    mitkSegmentationSubtaskHasValueMacro(name) \
    mitkSegmentationSubtaskGetValueMacro(type, name) \
    mitkSegmentationSubtaskSetValueMacro(type, name) \
  private: \
    std::optional<type> m_##name;

#define mitkSegmentationTaskGetValueMacro(type, name) \
  type Get##name(size_t index) const { \
    return index < m_Subtasks.size() ? m_Subtasks[index].Get##name() : type(); \
  }

#define mitkSegmentationTaskHasValueMacro(name) \
  bool Has##name(size_t index) const { \
    return index < m_Subtasks.size() && (m_Subtasks[index].Has##name() || m_Defaults.Has##name()); \
  }

#define mitkSegmentationTaskSetDefaultMacro(type, name) \
  void SetDefault##name(const type& value) { \
    m_Defaults.Set##name(value); \
  }

#define mitkSegmentationTaskValueMacro(type, name) \
  mitkSegmentationTaskHasValueMacro(name) \
  mitkSegmentationTaskGetValueMacro(type, name) \
  mitkSegmentationTaskSetDefaultMacro(type, name)

#endif
