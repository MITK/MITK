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

#include <usServiceReference.h>
#include <usServiceProperties.h>
#include <usAny.h>

namespace mitk {

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

FileWriterSelector::FileWriterSelector(const BaseData* baseData, const std::string& destMimeType)
  : m_Data(new Impl)
{
  mitk::CoreServicePointer<mitk::IMimeTypeProvider> mimeTypeProvider(mitk::CoreServices::GetMimeTypeProvider());

  // Get all writers and their mime types for the given base data type
  std::vector<FileWriterRegistry::WriterReference> refs = m_Data->m_WriterRegistry.GetReferences(baseData, destMimeType);
  us::ServiceReference<IFileWriter> bestRef;
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
          std::cout << "*** Checking confidence level of " << typeid(*writer).name() << " ... ";
          IFileWriter::ConfidenceLevel confidenceLevel = writer->GetConfidenceLevel(baseData);
          std::cout << confidenceLevel << std::endl;
          if (confidenceLevel == IFileWriter::Unsupported)
          {
            continue;
          }

          Item item;
          item.m_FileWriterRef = *iter;
          item.m_FileWriter = writer;
          item.m_ConfidenceLevel = confidenceLevel;
          item.m_MimeType = mimeType;
          item.m_Id = us::any_cast<long>(iter->GetProperty(us::ServiceConstants::SERVICE_ID()));
          m_Data->m_Items.insert(std::make_pair(item.m_Id, item));
          m_Data->m_MimeTypes.insert(mimeType);
          if (!bestRef || bestRef < *iter)
          {
            bestRef = *iter;
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

  if (bestRef)
  {
    try
    {
      m_Data->m_BestId = us::any_cast<long>(bestRef.GetProperty(us::ServiceConstants::SERVICE_ID()));
      m_Data->m_SelectedId = m_Data->m_BestId;
    } catch (const us::BadAnyCastException&) {}
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
  return Get(this->GetSelected().m_MimeType.GetName());
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
  return Select(item.m_Id);
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

IFileWriter* FileWriterSelector::Item::GetWriter() const
{
  return m_FileWriter;
}

std::string FileWriterSelector::Item::GetDescription() const
{
  us::Any descr = m_FileWriterRef.GetProperty(IFileWriter::PROP_DESCRIPTION());
  if (descr.Empty()) return std::string();
  return descr.ToString();
}

IFileWriter::ConfidenceLevel FileWriterSelector::Item::GetConfidenceLevel() const
{
  return m_ConfidenceLevel;
}

MimeType FileWriterSelector::Item::GetMimeType() const
{
  return m_MimeType;
}

us::ServiceReference<IFileWriter> FileWriterSelector::Item::GetReference() const
{
  return m_FileWriterRef;
}

long FileWriterSelector::Item::GetServiceId() const
{
  return m_Id;
}

bool FileWriterSelector::Item::operator<(const FileWriterSelector::Item& other) const
{
  // sort by confidence level first (ascending)
  if (m_ConfidenceLevel == other.m_ConfidenceLevel)
  {
    // sort by file writer service ranking
    return m_FileWriterRef < other.m_FileWriterRef;
  }
  return m_ConfidenceLevel < other.m_ConfidenceLevel;
}

FileWriterSelector::Item::Item()
  : m_FileWriter(NULL)
  , m_Id(-1)
{}

void swap(FileWriterSelector& fws1, FileWriterSelector& fws2)
{
  fws1.Swap(fws2);
}

}
