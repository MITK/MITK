/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USMODULERESOURCECONTAINER_P_H
#define USMODULERESOURCECONTAINER_P_H

#include <usGlobalConfig.h>
#include <cstdint>

#include <ctime>
#include <string>
#include <vector>

namespace us {

struct ModuleInfo;
class ModuleResource;
struct ModuleResourceContainerPrivate;

class ModuleResourceContainer
{

public:

  ModuleResourceContainer(const ModuleInfo* moduleInfo);
  ~ModuleResourceContainer();

  struct Stat
  {
    Stat()
      : index(-1)
      , uncompressedSize(0)
      , modifiedTime(0)
      , isDir(false)
    {}

    std::string filePath;
    int index;
    int uncompressedSize;
    time_t modifiedTime;
    bool isDir;
  };

  bool IsValid() const;

  bool GetStat(Stat& stat) const;
  bool GetStat(int index, Stat& stat) const;

  void* GetData(int index) const;

  const ModuleInfo* GetModuleInfo() const;

  void GetChildren(const std::string& resourcePath, bool relativePaths,
                   std::vector<std::string>& names, std::vector<uint32_t>& indices) const;

  void FindNodes(const std::string& path, const std::string& filePattern,
                 bool recurse, std::vector<ModuleResource>& resources) const;

private:

  bool Matches(const std::string& name, const std::string& filePattern) const;

  ModuleResourceContainerPrivate* d;

};


}

#endif // USMODULERESOURCECONTAINER_P_H
