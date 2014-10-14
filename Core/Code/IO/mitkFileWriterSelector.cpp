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

#include "mitkFileWriterSelector.h"

#include <mitkFileWriterRegistry.h>
#include <mitkCoreServices.h>
#include <mitkIMimeTypeProvider.h>
#include <mitkBaseData.h>

#include <usServiceReference.h>
#include <usServiceProperties.h>
#include <usAny.h>

#include <itksys/SystemTools.hxx>

#include <set>
#include <limits>
#include <iterator>

namespace mitk {

struct FileWriterSelector::Item::Impl : us::SharedData
{
  Impl()
    : m_FileWriter(NULL)
    , m_ConfidenceLevel(IFileWriter::Unsupported)
    , m_BaseDataIndex(0)
    , m_Id(-1)
  {}

  us::ServiceReference<IFileWriter> m_FileWriterRef;
  IFileWriter* m_FileWriter;
  IFileWriter::ConfidenceLevel m_ConfidenceLevel;
  std::size_t m_BaseDataIndex;
  MimeType m_MimeType;
  long m_Id;
};

struct FileWriterSelector::Impl : us::SharedData
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

  FileWriterRegistry m_WriterRegistry;
  std::map<long, FileWriterSelector::Item> m_Items;
  std::set<MimeType> m_MimeTypes;
  long m_BestId;
  long m_SelectedId;
};

FileWriterSelector::FileWriterSelector(const FileWriterSelector& other)
  : m_Data(other.m_Data)
{
}

FileWriterSelector::FileWriterSelector(const BaseData* baseData, const std::string& mimeType,
                                       const std::string& path)
  : m_Data(new Impl)
{
  mitk::CoreServicePointer<mitk::IMimeTypeProvider> mimeTypeProvider(mitk::CoreServices::GetMimeTypeProvider());

  std::string destMimeType = mimeType;
  if (destMimeType.empty() && !path.empty())
  {
    // try to derive a mime-type from the file
    std::vector<MimeType> mimeTypes = mimeTypeProvider->GetMimeTypesForFile(path);
    if (!mimeTypes.empty())
    {
      destMimeType = mimeTypes.front().GetName();
    }
    else if (!itksys::SystemTools::GetFilenameExtension(path).empty())
    {
      // If there are no suitable mime-type for the file AND an extension
      // was supplied, we stop here.
      return;
    }
  }

  std::vector<std::string> classHierarchy = baseData->GetClassHierarchy();

  // Get all writers and their mime types for the given base data type
  std::vector<FileWriterRegistry::WriterReference> refs = m_Data->m_WriterRegistry.GetReferences(baseData, destMimeType);
  Item bestItem;
  for (std::vector<FileWriterRegistry::WriterReference>::const_iterator iter = refs.begin(),
       iterEnd = refs.end(); iter != iterEnd; ++iter)
  {
    std::string mimeTypeName = iter->GetProperty(IFileWriter::PROP_MIMETYPE()).ToString();
    if (!mimeTypeName.empty())
    {
      MimeType mimeType = mimeTypeProvider->GetMimeTypeForName(mimeTypeName);
      if (mimeType.IsValid())
      {
        // There is a registered mime-type for this writer. Now get the confidence level
        // of this writer for writing the given base data object.

        IFileWriter* writer = m_Data->m_WriterRegistry.GetWriter(*iter);
        if (writer == NULL) continue;
        try
        {
          writer->SetInput(baseData);
          IFileWriter::ConfidenceLevel confidenceLevel = writer->GetConfidenceLevel();
          if (confidenceLevel == IFileWriter::Unsupported)
          {
            continue;
          }

          std::string baseDataType = iter->GetProperty(IFileWriter::PROP_BASEDATA_TYPE()).ToString();
          std::vector<std::string>::iterator idxIter =
              std::find(classHierarchy.begin(), classHierarchy.end(), baseDataType);
          std::size_t baseDataIndex = std::numeric_limits<std::size_t>::max();
          if (idxIter != classHierarchy.end())
          {
            baseDataIndex = std::distance(classHierarchy.begin(), idxIter);
          }

          Item item;
          item.d->m_FileWriterRef = *iter;
          item.d->m_FileWriter = writer;
          item.d->m_ConfidenceLevel = confidenceLevel;
          item.d->m_BaseDataIndex = baseDataIndex;
          item.d->m_MimeType = mimeType;
          item.d->m_Id = us::any_cast<long>(iter->GetProperty(us::ServiceConstants::SERVICE_ID()));
          m_Data->m_Items.insert(std::make_pair(item.d->m_Id, item));
          m_Data->m_MimeTypes.insert(mimeType);
          if (!bestItem.GetReference() || bestItem < item)
          {
            bestItem = item;
          }
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
  }

  if (bestItem.GetReference())
  {
    m_Data->m_BestId = bestItem.GetServiceId();
    m_Data->m_SelectedId = m_Data->m_BestId;
  }
}

FileWriterSelector::~FileWriterSelector()
{
}

FileWriterSelector& FileWriterSelector::operator=(const FileWriterSelector& other)
{
  m_Data = other.m_Data;
  return *this;
}

bool FileWriterSelector::IsEmpty() const
{
  return m_Data->m_Items.empty();
}

std::vector<FileWriterSelector::Item> FileWriterSelector::Get(const std::string& mimeType) const
{
  std::vector<Item> result;
  for (std::map<long, Item>::const_iterator iter = m_Data->m_Items.begin(),
       iterEnd = m_Data->m_Items.end(); iter != iterEnd; ++iter)
  {
    if (mimeType.empty() || iter->second.GetMimeType().GetName() == mimeType)
    {
      result.push_back(iter->second);
    }
  }
  std::sort(result.begin(), result.end());
  return result;
}

std::vector<FileWriterSelector::Item> FileWriterSelector::Get() const
{
  return Get(this->GetSelected().d->m_MimeType.GetName());
}

FileWriterSelector::Item FileWriterSelector::Get(long id) const
{
  std::map<long, Item>::const_iterator iter = m_Data->m_Items.find(id);
  if (iter != m_Data->m_Items.end())
  {
    return iter->second;
  }
  return Item();
}

FileWriterSelector::Item FileWriterSelector::GetDefault() const
{
  return Get(m_Data->m_BestId);
}

long FileWriterSelector::GetDefaultId() const
{
  return m_Data->m_BestId;
}

FileWriterSelector::Item FileWriterSelector::GetSelected() const
{
  return Get(m_Data->m_SelectedId);
}

long FileWriterSelector::GetSelectedId() const
{
  return m_Data->m_SelectedId;
}

bool FileWriterSelector::Select(const std::string& mimeType)
{
  std::vector<Item> items = Get(mimeType);
  if (items.empty()) return false;
  return Select(items.back());
}

bool FileWriterSelector::Select(const FileWriterSelector::Item& item)
{
  return Select(item.d->m_Id);
}

bool FileWriterSelector::Select(long id)
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

std::vector<MimeType> FileWriterSelector::GetMimeTypes() const
{
  std::vector<MimeType> result;
  result.reserve(m_Data->m_MimeTypes.size());
  result.assign(m_Data->m_MimeTypes.begin(), m_Data->m_MimeTypes.end());
  return result;
}

void FileWriterSelector::Swap(FileWriterSelector& fws)
{
  m_Data.Swap(fws.m_Data);
}

FileWriterSelector::Item::Item(const FileWriterSelector::Item& other)
  : d(other.d)
{
}

FileWriterSelector::Item::~Item()
{
}

FileWriterSelector::Item& FileWriterSelector::Item::operator=(const FileWriterSelector::Item& other)
{
  d = other.d;
  return *this;
}

IFileWriter* FileWriterSelector::Item::GetWriter() const
{
  return d->m_FileWriter;
}

std::string FileWriterSelector::Item::GetDescription() const
{
  us::Any descr = d->m_FileWriterRef.GetProperty(IFileWriter::PROP_DESCRIPTION());
  if (descr.Empty()) return std::string();
  return descr.ToString();
}

IFileWriter::ConfidenceLevel FileWriterSelector::Item::GetConfidenceLevel() const
{
  return d->m_ConfidenceLevel;
}

MimeType FileWriterSelector::Item::GetMimeType() const
{
  return d->m_MimeType;
}

std::string FileWriterSelector::Item::GetBaseDataType() const
{
  us::Any any = d->m_FileWriterRef.GetProperty(IFileWriter::PROP_BASEDATA_TYPE());
  if (any.Empty()) return std::string();
  return any.ToString();
}

us::ServiceReference<IFileWriter> FileWriterSelector::Item::GetReference() const
{
  return d->m_FileWriterRef;
}

long FileWriterSelector::Item::GetServiceId() const
{
  return d->m_Id;
}

bool FileWriterSelector::Item::operator<(const FileWriterSelector::Item& other) const
{
  // sort by confidence level first (ascending)
  if (d->m_ConfidenceLevel == other.d->m_ConfidenceLevel)
  {
    // sort by class hierarchy index (writers for more derived
    // based data types are considered a better match)
    if (d->m_BaseDataIndex == other.d->m_BaseDataIndex)
    {
      // sort by file writer service ranking
      return d->m_FileWriterRef < other.d->m_FileWriterRef;
    }
    return other.d->m_BaseDataIndex < d->m_BaseDataIndex;
  }
  return d->m_ConfidenceLevel < other.d->m_ConfidenceLevel;
}

FileWriterSelector::Item::Item()
  : d(new Impl())
{}

void swap(FileWriterSelector& fws1, FileWriterSelector& fws2)
{
  fws1.Swap(fws2);
}

}
