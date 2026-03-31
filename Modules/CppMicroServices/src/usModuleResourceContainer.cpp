/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "usModuleResourceContainer_p.h"

#include <usModuleInfo.h>
#include <usModuleUtils_p.h>
#include <usModuleResource.h>
#include <mitkLog.h>

#include "us_miniz.h"

#include <set>
#include <cstring>
#include <climits>
#include <cassert>

namespace us {

struct ModuleResourceContainerPrivate
{
  ModuleResourceContainerPrivate(const ModuleInfo* moduleInfo)
    : m_ModuleInfo(moduleInfo)
    , m_IsValid(false)
    , m_ZipArchive()
  {}

  typedef std::pair<std::string, int> NameIndexPair;

  struct PairComp
  {
    bool operator()(const NameIndexPair& p1, const NameIndexPair& p2) const
    {
      return p1.first < p2.first;
    }
  };

  typedef std::set<NameIndexPair, PairComp> SetType;

  void InitSortedEntries()
  {
    if (m_SortedEntries.empty())
    {
      us_mz_uint numFiles = us_mz_zip_reader_get_num_files(&m_ZipArchive);
      for (us_mz_uint fileIndex = 0; fileIndex < numFiles; ++fileIndex)
      {
        char fileName[US_MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE];
        us_mz_zip_reader_get_filename(&m_ZipArchive, fileIndex, fileName, US_MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE);
        m_SortedEntries.insert(std::make_pair(std::string(fileName), fileIndex));
      }
    }
  }

  const ModuleInfo* m_ModuleInfo;
  bool m_IsValid;

  us_mz_zip_archive m_ZipArchive;

  std::set<NameIndexPair, PairComp> m_SortedEntries;
};

ModuleResourceContainer::ModuleResourceContainer(const ModuleInfo* moduleInfo)
  : d(new ModuleResourceContainerPrivate(moduleInfo))
{
  if (us_mz_zip_reader_init_file(&d->m_ZipArchive, moduleInfo->location.c_str(), 0))
  {
    d->m_IsValid = true;
  }
  else
  {
  }
}

ModuleResourceContainer::~ModuleResourceContainer()
{
  if (IsValid())
  {
    us_mz_zip_reader_end(&d->m_ZipArchive);
  }
  delete d;
}

bool ModuleResourceContainer::IsValid() const
{
  return d->m_IsValid;
}

bool ModuleResourceContainer::GetStat(ModuleResourceContainer::Stat& stat) const
{
  if (IsValid())
  {
    int fileIndex = us_mz_zip_reader_locate_file(&d->m_ZipArchive, stat.filePath.c_str(), nullptr, 0);
    if (fileIndex >= 0)
    {
      return GetStat(fileIndex, stat);
    }
  }
  return false;
}

bool ModuleResourceContainer::GetStat(int index, ModuleResourceContainer::Stat& stat) const
{
  if (IsValid())
  {
    if (index >= 0)
    {
      us_mz_zip_archive_file_stat zipStat;
      if (!us_mz_zip_reader_file_stat(&d->m_ZipArchive, index, &zipStat))
      {
        return false;
      }
      stat.index = index;
      stat.filePath = zipStat.m_filename;
      stat.isDir = us_mz_zip_reader_is_file_a_directory(&d->m_ZipArchive, index) ? true : false;
      stat.modifiedTime = zipStat.m_time;
      // This will limit the size info from uint64 to uint32 on 32-bit
      // architectures. We don't care because we assume resources > 2GB
      // don't make sense to be embedded in a module anyway.
      assert(zipStat.m_comp_size < INT_MAX);
      assert(zipStat.m_uncomp_size < INT_MAX);
      stat.uncompressedSize = static_cast<int>(zipStat.m_uncomp_size);
      return true;
    }
  }
  return false;
}

void* ModuleResourceContainer::GetData(int index) const
{
  return us_mz_zip_reader_extract_to_heap(&d->m_ZipArchive, index, nullptr, 0);
}

const ModuleInfo*ModuleResourceContainer::GetModuleInfo() const
{
  return d->m_ModuleInfo;
}

void ModuleResourceContainer::GetChildren(const std::string& resourcePath, bool relativePaths,
                                          std::vector<std::string>& names, std::vector<uint32_t>& indices) const
{
  d->InitSortedEntries();

  ModuleResourceContainerPrivate::SetType::const_iterator iter =
      d->m_SortedEntries.find(std::make_pair(resourcePath, 0));
  if (iter == d->m_SortedEntries.end())
  {
    return;
  }

  for (++iter; iter != d->m_SortedEntries.end(); ++iter)
  {
    if (resourcePath.size() > iter->first.size()) break;
    if (iter->first.compare(0, resourcePath.size(), resourcePath) == 0)
    {
      std::size_t pos = iter->first.find_first_of('/', resourcePath.size());
      if (pos == std::string::npos || pos == iter->first.size()-1)
      {
        if (relativePaths)
        {
          names.push_back(iter->first.substr(resourcePath.size()));
        }
        else
        {
          names.push_back(iter->first);
        }
        indices.push_back(iter->second);
      }
    }
  }
}

void ModuleResourceContainer::FindNodes(const std::string& path, const std::string& filePattern,
                                        bool recurse, std::vector<ModuleResource>& resources) const
{
  std::vector<std::string> names;
  std::vector<uint32_t> indices;

  this->GetChildren(path, true, names, indices);

  for(std::size_t i = 0, s = names.size(); i < s; ++i)
  {
    if (*names[i].rbegin() == '/' && recurse)
    {
      this->FindNodes(path + names[i], filePattern, recurse, resources);
    }
    if (this->Matches(names[i], filePattern))
    {
      resources.push_back(ModuleResource(indices[i], *this));
    }
  }
}

bool ModuleResourceContainer::Matches(const std::string& name, const std::string& filePattern) const
{
  // short-cut
  if (filePattern == "*") return true;

  std::vector<std::string> tokens;
  std::stringstream ss(filePattern);
  std::string tok;
  while(std::getline(ss, tok, '*'))
  {
    tokens.push_back(tok);
  }

  if (tokens.empty()) return true;

  bool startsWithWild = !filePattern.empty() && filePattern[0] == '*';
  bool endsWithWild = !filePattern.empty() && filePattern[filePattern.size()-1] == '*';

  std::size_t pos = 0;
  for (std::size_t i = 0; i < tokens.size(); ++i)
  {
    if (tokens[i].empty()) continue;

    if (i == 0 && !startsWithWild)
    {
      // First token must match at the start of name
      if (name.compare(0, tokens[i].size(), tokens[i]) != 0) return false;
      pos = tokens[i].size();
    }
    else
    {
      std::size_t index = name.find(tokens[i], pos);
      if (index == std::string::npos) return false;
      pos = index + tokens[i].size();
    }
  }

  // Last token must match at the end of name (unless pattern ends with *)
  if (!endsWithWild && pos != name.size()) return false;

  return true;
}

}
