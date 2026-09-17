/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegSourceReferenceTestHelpers_h
#define mitkSegSourceReferenceTestHelpers_h

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcitem.h>
#include <dcmtk/dcmdata/dcsequen.h>
#include <dcmtk/ofstd/ofstring.h>

#include <set>
#include <sstream>
#include <string>

namespace mitk
{
  namespace test
  {
    /**
     * \brief The value of a top-level DICOM tag of the file at \c path.
     *
     * \return The tag's value as a string. An unreadable file, an absent
     *         dataset or a missing tag all yield an empty string, so a
     *         caller that compares against an expected value fails rather
     *         than reading a stale or partial result.
     */
    inline std::string ReadTag(const std::string &path, const DcmTagKey &tag)
    {
      DcmFileFormat ff;
      if (ff.loadFile(path.c_str()).bad() || ff.getDataset() == nullptr)
        return {};

      OFString value;
      if (ff.getDataset()->findAndGetOFString(tag, value).bad())
        return {};

      return value.c_str();
    }

    /**
     * \brief Collect every ReferencedSOPInstanceUID below \c item, at any
     *        nesting depth.
     *
     * dcmqi places source references both in the top-level
     * ReferencedSeriesSequence and in the per-frame derivation groups. Tests
     * that grade the provenance of a written SEG care which instances it
     * references, not where the references sit, so the walk is exhaustive
     * rather than path-specific.
     */
    inline void CollectReferencedSOPInstanceUIDs(DcmItem *item, std::set<std::string> &out)
    {
      if (item == nullptr)
        return;

      for (unsigned long i = 0; i < item->card(); ++i)
      {
        DcmElement *element = item->getElement(i);
        if (element == nullptr)
          continue;

        if (element->getTag() == DCM_ReferencedSOPInstanceUID)
        {
          OFString value;
          if (element->getOFString(value, 0).good())
            out.insert(value.c_str());
        }
        else if (auto *sequence = dynamic_cast<DcmSequenceOfItems *>(element))
        {
          for (unsigned long j = 0; j < sequence->card(); ++j)
            CollectReferencedSOPInstanceUIDs(sequence->getItem(j), out);
        }
      }
    }

    /**
     * \brief Every SOP Instance UID the DICOM SEG at \c segPath references.
     *
     * \return The referenced UIDs. An unreadable file yields an empty set, so
     *         it fails the caller's assertion the same way a SEG that
     *         references nothing does.
     */
    inline std::set<std::string> ReadReferencedSOPInstanceUIDs(const std::string &segPath)
    {
      std::set<std::string> result;

      DcmFileFormat ff;
      if (ff.loadFile(segPath.c_str()).good())
        CollectReferencedSOPInstanceUIDs(ff.getDataset(), result);

      return result;
    }

    /**
     * \brief Compare the instances a written SEG references against the
     *        instances its source declares.
     *
     * \return An empty string when the two sets are equal, otherwise a
     *         diagnostic naming every source instance that went unreferenced
     *         and every referenced instance the source does not declare. Both
     *         directions matter: a SEG that references the real instances and
     *         a minted series besides misstates its provenance just as one
     *         that references nothing does.
     */
    inline std::string DiffSourceReferences(const std::set<std::string> &expected,
                                            const std::set<std::string> &referenced)
    {
      std::ostringstream diff;

      for (const auto &uid : expected)
      {
        if (referenced.find(uid) == referenced.end())
          diff << "\n  source instance not referenced: " << uid;
      }

      for (const auto &uid : referenced)
      {
        if (expected.find(uid) == expected.end())
          diff << "\n  instance referenced but not in the source: " << uid;
      }

      return diff.str();
    }
  }
}

#endif
