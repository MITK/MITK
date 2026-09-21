/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkDICOMGenericImageFrameInfo.h>

#include <mitkFileSystem.h>

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcpath.h>
#include <dcmtk/dcmdata/dcsequen.h>

#include <algorithm>
#include <optional>
#include <vector>

namespace
{
  std::optional<mitk::DICOMFrameLayout::Rescale> ReadPixelValueTransformation(DcmItem& group)
  {
    DcmItem* transformation = nullptr;
    if (group.findAndGetSequenceItem(DCM_PixelValueTransformationSequence, transformation, 0).bad()
        || nullptr == transformation)
    {
      return std::nullopt;
    }

    mitk::DICOMFrameLayout::Rescale rescale;
    Float64 value = 0.0;
    if (transformation->findAndGetFloat64(DCM_RescaleSlope, value).good())
    {
      rescale.slope = value;
    }
    if (transformation->findAndGetFloat64(DCM_RescaleIntercept, value).good())
    {
      rescale.intercept = value;
    }

    return rescale;
  }

  mitk::DICOMFrameLayout ReadFrameLayout(DcmDataset& dataset,
                                         const std::string& filename,
                                         std::vector<mitk::DICOMFrameModelFinding>& warningsThisScan)
  {
    mitk::DICOMFrameLayout layout;

    // Per-frame groups first, and almost nothing else when they are absent.
    // DcmItem::search walks the whole top-level element list with no early break
    // on tag order, so an absent tag costs a full walk and the number of lookups
    // is what a single-frame input pays for this feature.
    DcmSequenceOfItems* perFrame = nullptr;
    const bool hasPerFrameGroups =
      dataset.findAndGetSequence(DCM_PerFrameFunctionalGroupsSequence, perFrame).good() && nullptr != perFrame;

    Sint32 frames = 1;
    if (dataset.findAndGetSint32(DCM_NumberOfFrames, frames).good() && frames > 1)
    {
      layout.frameCount = static_cast<unsigned int>(frames);
    }

    if (!hasPerFrameGroups)
    {
      // Two walks and out. The frame count is still read because the diagnostics
      // report names it for a plain multi-frame object.
      return layout;
    }

    layout.perFrameItemCount = static_cast<unsigned int>(perFrame->card());
    layout.perFrameRescale.assign(layout.perFrameItemCount, std::nullopt);
    for (unsigned int k = 0; k < layout.perFrameItemCount; ++k)
    {
      if (DcmItem* item = perFrame->getItem(k))
      {
        layout.perFrameRescale[k] = ReadPixelValueTransformation(*item);
      }
    }

    if (std::none_of(layout.perFrameRescale.cbegin(), layout.perFrameRescale.cend(),
                     [](const auto& rescale) { return rescale.has_value(); }))
    {
      layout.perFrameRescale.clear();
    }

    // Reached only for a file that has per-frame groups, which is also the only
    // file whose shared rescale anything reads.
    DcmItem* shared = nullptr;
    if (dataset.findAndGetSequenceItem(DCM_SharedFunctionalGroupsSequence, shared, 0).good() && nullptr != shared)
    {
      layout.sharedRescale = ReadPixelValueTransformation(*shared);
    }

    for (const auto& finding : mitk::CollectFrameModelFindings(layout, filename))
    {
      if (mitk::DICOMFrameModelSeverity::Warning == finding.severity)
      {
        warningsThisScan.push_back(finding);
      }
    }

    return layout;
  }

  /** One line per distinct condition rather than one per file: a directory of
      non-conformant files would otherwise produce an identical line per file. */
  void ReportWarnings(const std::vector<mitk::DICOMFrameModelFinding>& warnings)
  {
    for (const auto issue : mitk::AllDICOMFrameModelIssues())
    {
      const auto first = std::find_if(warnings.cbegin(), warnings.cend(),
                                      [issue](const auto& finding) { return finding.issue == issue; });
      if (warnings.cend() == first)
      {
        continue;
      }

      const auto count = std::count_if(warnings.cbegin(), warnings.cend(),
                                       [issue](const auto& finding) { return finding.issue == issue; });

      MITK_WARN << mitk::DICOMFrameModelIssueToString(issue)
                << " Frames: " << first->frameCount
                << ", per-frame items: " << first->perFrameItemCount
                << ". First file: " << first->files.front()
                << (count > 1 ? " (and " + std::to_string(count - 1) + " more)" : "");
    }
  }
}

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
    std::vector<DICOMFrameModelFinding> warningsThisScan;

    for (const auto& fileName : this->m_InputFilenames)
    {
      if (fs::is_directory(fileName))
        continue;

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
        // Before AddFrameInfo, which reads the layout to maintain the
        // cache-level frame-model flag.
        info->SetFrameLayout(ReadFrameLayout(*dfile.getDataset(), fileName, warningsThisScan));
        newCache->AddFrameInfo(info);
      }
    }

    ReportWarnings(warningsThisScan);

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
