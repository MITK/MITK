/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSceneJsonMimeType_h
#define mitkSceneJsonMimeType_h

#include <mitkCustomMimeType.h>

namespace mitk
{
  /**
   * \brief Custom MIME type for MITK JSON scene files (.mitkscene.json).
   *
   * Registered for file-type detection, dialog filters and file associations
   * only. No AbstractFileReader is registered behind it: scene loading is
   * reached through SceneIO, not IOUtil. See AbstractSceneReader.
   */
  class SceneJsonMimeType : public CustomMimeType
  {
  public:
    SceneJsonMimeType();

    bool AppliesTo(const std::string &path) const override;
    SceneJsonMimeType *Clone() const override;

    static std::string Name();
  };
}

#endif
