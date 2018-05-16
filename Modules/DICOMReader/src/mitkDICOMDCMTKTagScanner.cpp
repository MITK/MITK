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

#include "mitkDICOMDCMTKTagScanner.h"
#include "mitkDICOMGenericImageFrameInfo.h"

#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcpath.h>

mitk::DICOMDCMTKTagScanner::DICOMDCMTKTagScanner()
{
}

mitk::DICOMDCMTKTagScanner::~DICOMDCMTKTagScanner()
{
}

void mitk::DICOMDCMTKTagScanner::AddTag( const DICOMTag& tag )
{
  m_ScannedTags.insert( DICOMTagPath(tag) );
}

void mitk::DICOMDCMTKTagScanner::AddTags( const DICOMTagList& tags )
{
  for ( auto tagIter = tags.cbegin(); tagIter != tags.cend(); ++tagIter )
  {
    this->AddTag( *tagIter );
  }
}

void mitk::DICOMDCMTKTagScanner::AddTagPath(const DICOMTagPath& path)
{
  m_ScannedTags.insert(path);
}

void mitk::DICOMDCMTKTagScanner::AddTagPaths(const DICOMTagPathList& paths)
{
  for (const auto& path : paths)
  {
    this->AddTagPath(path);
  }
}

void mitk::DICOMDCMTKTagScanner::SetInputFiles( const StringList& filenames )
{
  m_InputFilenames = filenames;
}

mitk::DICOMTagPath DcmPathToTagPath(DcmPath * dcmpath)
{
  mitk::DICOMTagPath result;

  OFListConstIterator(DcmPathNode*) it = dcmpath->begin();
  OFListConstIterator(DcmPathNode*) endOfList = dcmpath->end();
  OFString pathStr; DcmEVR vr; DcmObject* obj;

  while (it != endOfList)
  {
    if (((*it) == nullptr) || ((*it)->m_obj == nullptr))
    {
      mitkThrow() << "Error in DcmPathToTagPath(). Invalid search result";
    }
    obj = (*it)->m_obj;
    vr = obj->ident();

    if ((vr == EVR_SQ) || (obj->isLeaf()))
    {
      result.AddElement(obj->getTag().getGroup(), obj->getTag().getElement());
    }
    else if ((vr == EVR_item) || (vr == EVR_dataset))
    {
      if (result.Size() > 0)
      {
        result.GetLastNode().type = mitk::DICOMTagPath::NodeInfo::NodeType::SequenceSelection;
        result.GetLastNode().selection = (*it)->m_itemNo;
      }
      else
      {
        mitkThrow() << "Error in DcmPathToTagPath(). DCMTK path is illegal due to toplevel sequence item.";
      }
    }
    else
    {
      result.AddNode(mitk::DICOMTagPath::NodeInfo());
    }
    ++it;
  }

  return result;
}

void mitk::DICOMDCMTKTagScanner::Scan()
{
  this->PushLocale();

  try
  {
    DcmPathProcessor processor;
    processor.setItemWildcardSupport(true);

    DICOMGenericTagCache::Pointer newCache = DICOMGenericTagCache::New();

    for (const auto& fileName : this->m_InputFilenames)
    {
      DcmFileFormat dfile;
      OFCondition cond = dfile.loadFile(fileName.c_str());
      if (cond.bad())
      {
        MITK_ERROR << "Error when scanning for tags. Cannot open given file. File: " << fileName;
      }
      else
      {
        DICOMGenericImageFrameInfo::Pointer info = DICOMGenericImageFrameInfo::New(fileName);

        for (const auto& path : this->m_ScannedTags)
        {
          std::string tagPath = DICOMTagPathToDCMTKSearchPath(path);
          cond = processor.findOrCreatePath(dfile.getDataset(), tagPath.c_str());
          if (cond.good())
          {
            OFList< DcmPath * > findings;
            processor.getResults(findings);
            for (const auto& finding : findings)
            {
              auto element = dynamic_cast<DcmElement*>(finding->back()->m_obj);
              if (!element)
              {
                auto item = dynamic_cast<DcmItem*>(finding->back()->m_obj);
                if (item)
                {
                  element = item->getElement(finding->back()->m_itemNo);
                }
              }

              if (element)
              {
                OFString value;
                cond = element->getOFStringArray(value);
                if (cond.good())
                {
                  info->SetTagValue(DcmPathToTagPath(finding), std::string(value.c_str()));
                }
              }
            }
          }
        }
        newCache->AddFrameInfo(info);
      }
    }

    m_Cache = newCache;

    this->PopLocale();
  }
  catch (...)
  {
    this->PopLocale();
    throw;
  }
}

mitk::DICOMTagCache::Pointer
mitk::DICOMDCMTKTagScanner::GetScanCache() const
{
  return m_Cache.GetPointer();
}

mitk::DICOMDatasetAccessingImageFrameList mitk::DICOMDCMTKTagScanner::GetFrameInfoList() const
{
  if (m_Cache.IsNotNull())
  {
    return m_Cache->GetFrameInfoList();
  }
  return mitk::DICOMDatasetAccessingImageFrameList();
}
