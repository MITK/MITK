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

#include "mitkFileReaderSelector.h"

#include <mitkFileReaderRegistry.h>
#include <mitkCoreServices.h>
#include <mitkIMimeTypeProvider.h>

#include <usServiceReference.h>
#include <usServiceProperties.h>
#include <usAny.h>

#include <itksys/SystemTools.hxx>

namespace mitk {

struct FileReaderSelector::Item::Impl : us::SharedData
{
  Impl()
    : m_FileReader(NULL)
    , m_ConfidenceLevel(IFileReader::Unsupported)
    , m_Id(-1)
  {}

  us::ServiceReference<IFileReader> m_FileReaderRef;
  IFileReader* m_FileReader;
  IFileReader::ConfidenceLevel m_ConfidenceLevel;
  MimeType m_MimeType;
  long m_Id;
};

struct FileReaderSelector::Impl : us::SharedData
{
  Impl()
    : m_BestId(-1)
    , m_SelectedId(m_BestId)
  {}

  Impl(const Impl& other)
    : us::SharedData(other)
    , m_BestId(-1)
    , m_SelectedId(m_BestId)
  {}

  FileReaderRegistry m_ReaderRegistry;
  std::map<long, FileReaderSelector::Item> m_Items;
  std::vector<MimeType> m_MimeTypes;
  long m_BestId;
  long m_SelectedId;
};

FileReaderSelector::FileReaderSelector(const FileReaderSelector& other)
  : m_Data(other.m_Data)
{
}

FileReaderSelector::FileReaderSelector(const std::string& path)
  : m_Data(new Impl)
{
  if (!itksys::SystemTools::FileExists(path.c_str()))
  {
    return;
  }

  mitk::CoreServicePointer<mitk::IMimeTypeProvider> mimeTypeProvider(mitk::CoreServices::GetMimeTypeProvider());

  // Get all mime types and associated readers for the given file path

  m_Data->m_MimeTypes = mimeTypeProvider->GetMimeTypesForFile(path);
  if (m_Data->m_MimeTypes.empty()) return;

  for (std::vector<MimeType>::const_iterator mimeTypeIter = m_Data->m_MimeTypes.begin(),
       mimeTypeIterEnd = m_Data->m_MimeTypes.end(); mimeTypeIter != mimeTypeIterEnd; ++mimeTypeIter)
  {
    std::vector<FileReaderRegistry::ReaderReference> refs = m_Data->m_ReaderRegistry.GetReferences(*mimeTypeIter);
    for (std::vector<FileReaderRegistry::ReaderReference>::const_iterator readerIter = refs.begin(),
         iterEnd = refs.end(); readerIter != iterEnd; ++readerIter)
    {
      IFileReader* reader = m_Data->m_ReaderRegistry.GetReader(*readerIter);
      if (reader == NULL) continue;
      try
      {
        reader->SetInput(path);
        IFileReader::ConfidenceLevel confidenceLevel = reader->GetConfidenceLevel();
        if (confidenceLevel == IFileReader::Unsupported)
        {
          continue;
        }

        Item item;
        item.d->m_FileReaderRef = *readerIter;
        item.d->m_FileReader = reader;
        item.d->m_ConfidenceLevel = confidenceLevel;
        item.d->m_MimeType = *mimeTypeIter;
        item.d->m_Id = us::any_cast<long>(readerIter->GetProperty(us::ServiceConstants::SERVICE_ID()));
        m_Data->m_Items.insert(std::make_pair(item.d->m_Id, item));
        //m_Data->m_MimeTypes.insert(mimeType);
      }
      catch (const us::BadAnyCastException& e)
      {
        MITK_WARN << "Unexpected: " << e.what();
      }
      catch (const std::exception& e)
      {
        // Log the error but continue
        MITK_WARN << "IFileWriter::GetConfidenceLevel exception: " << e.what();
      }
    }
  }

  // get the "best" reader

  if (!m_Data->m_Items.empty())
  {
    std::set<Item> sortedItems;
    for (std::map<long, Item>::const_iterator iter = m_Data->m_Items.begin(),
         iterEnd = m_Data->m_Items.end(); iter != iterEnd; ++iter)
    {
      sortedItems.insert(iter->second);
    }
    m_Data->m_BestId = sortedItems.rbegin()->GetServiceId();
    m_Data->m_SelectedId = m_Data->m_BestId;
  }
}

FileReaderSelector::~FileReaderSelector()
{
}

FileReaderSelector& FileReaderSelector::operator=(const FileReaderSelector& other)
{
  m_Data = other.m_Data;
  return *this;
}

bool FileReaderSelector::IsEmpty() const
{
  return m_Data->m_Items.empty();
}

std::vector<MimeType> FileReaderSelector::GetMimeTypes() const
{
  return m_Data->m_MimeTypes;
}

std::vector<FileReaderSelector::Item> FileReaderSelector::Get() const
{
  std::vector<Item> result;
  for (std::map<long, Item>::const_iterator iter = m_Data->m_Items.begin(),
       iterEnd = m_Data->m_Items.end(); iter != iterEnd; ++iter)
  {
    result.push_back(iter->second);
  }
  std::sort(result.begin(), result.end());
  return result;
}

FileReaderSelector::Item FileReaderSelector::Get(long id) const
{
  std::map<long, Item>::const_iterator iter = m_Data->m_Items.find(id);
  if (iter != m_Data->m_Items.end())
  {
    return iter->second;
  }
  return Item();
}

FileReaderSelector::Item FileReaderSelector::GetDefault() const
{
  return Get(m_Data->m_BestId);
}

long FileReaderSelector::GetDefaultId() const
{
  return m_Data->m_BestId;
}

FileReaderSelector::Item FileReaderSelector::GetSelected() const
{
  return Get(m_Data->m_SelectedId);
}

long FileReaderSelector::GetSelectedId() const
{
  return m_Data->m_SelectedId;
}

bool FileReaderSelector::Select(const FileReaderSelector::Item& item)
{
  return Select(item.d->m_Id);
}

bool FileReaderSelector::Select(long id)
{
  if (id > -1)
  {
    if (m_Data->m_Items.find(id) == m_Data->m_Items.end())
    {
      return false;
    }
    m_Data->m_SelectedId = id;
    return true;
  }
  return false;
}

void FileReaderSelector::Swap(FileReaderSelector& fws)
{
  m_Data.Swap(fws.m_Data);
}

FileReaderSelector::Item::Item(const FileReaderSelector::Item& other)
  : d(other.d)
{
}

FileReaderSelector::Item::~Item()
{
}

FileReaderSelector::Item& FileReaderSelector::Item::operator=(const FileReaderSelector::Item& other)
{
  d = other.d;
  return *this;
}

IFileReader* FileReaderSelector::Item::GetReader() const
{
  return d->m_FileReader;
}

std::string FileReaderSelector::Item::GetDescription() const
{
  us::Any descr = d->m_FileReaderRef.GetProperty(IFileReader::PROP_DESCRIPTION());
  if (descr.Empty()) return std::string();
  return descr.ToString();
}

IFileReader::ConfidenceLevel FileReaderSelector::Item::GetConfidenceLevel() const
{
  return d->m_ConfidenceLevel;
}

MimeType FileReaderSelector::Item::GetMimeType() const
{
  return d->m_MimeType;
}

us::ServiceReference<IFileReader> FileReaderSelector::Item::GetReference() const
{
  return d->m_FileReaderRef;
}

long FileReaderSelector::Item::GetServiceId() const
{
  return d->m_Id;
}

bool FileReaderSelector::Item::operator<(const FileReaderSelector::Item& other) const
{
  // sort by confidence level first (ascending)
  if (d->m_ConfidenceLevel == other.d->m_ConfidenceLevel)
  {
    // sort by mime-type ranking
    if (d->m_MimeType < other.d->m_MimeType)
    {
      return true;
    }
    else if (other.d->m_MimeType < d->m_MimeType)
    {
      return false;
    }
    else
    {
      // sort by file writer service ranking
      return d->m_FileReaderRef < other.d->m_FileReaderRef;
    }
  }
  return d->m_ConfidenceLevel < other.d->m_ConfidenceLevel;
}

FileReaderSelector::Item::Item()
  : d(new Impl())
{}

void swap(FileReaderSelector& frs1, FileReaderSelector& frs2)
{
  frs1.Swap(frs2);
}

}
