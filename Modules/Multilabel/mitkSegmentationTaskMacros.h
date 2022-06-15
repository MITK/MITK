/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentationTaskMacros_h
#define mitkSegmentationTaskMacros_h

#define mitkSubtaskGetMacro(x) \
  std::string Get##x() const { \
    if (!m_##x.empty()) return m_##x; \
    if (m_Defaults != nullptr) return m_Defaults->Get##x(); \
    return ""; \
  }

#define mitkSubtaskSetMacro(x) \
  void Set##x(const std::string& value) { \
    m_##x = value; \
  }

#define mitkSubtaskValueMacro(x) \
  public: \
    mitkSubtaskGetMacro(x) \
    mitkSubtaskSetMacro(x) \
  private: \
    std::string m_##x;

#define mitkTaskGetMacro(x) \
  std::string Get##x(size_t index) const { \
    return index < m_Subtasks.size() ? m_Subtasks[index].Get##x() : ""; \
  }

#define mitkTaskSetDefaultMacro(x) \
  void SetDefault##x(const std::string& value) { \
    m_Defaults.Set##x(value); \
  }

#define mitkTaskValueMacro(x) \
  mitkTaskGetMacro(x) \
  mitkTaskSetDefaultMacro(x)

#endif
