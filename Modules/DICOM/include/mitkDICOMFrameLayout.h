/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMFrameLayout_h
#define mitkDICOMFrameLayout_h

#include <MitkDICOMExports.h>

#include <optional>
#include <string>
#include <vector>

namespace mitk
{
  /**
   * \ingroup DICOMModule
   * \brief Layout of one multi-frame file as far as the frame model needs it.
   *
   * Recorded by the scanner that parses the file and handed out by its tag
   * cache. A cache that cannot look into sequences, or whose scanner was not
   * asked to read the frame model, reports the default, which is the one-frame
   * model every single-frame file has.
   *
   * \sa DICOMTagCache::GetFrameLayout, DICOMDCMTKTagScanner
   */
  struct DICOMFrameLayout
  {
    /** \brief A Pixel Value Transformation pair, (0028,1053) over (0028,1052). */
    struct Rescale
    {
      double slope = 1.0;
      double intercept = 0.0;
    };

    /** (0028,0008) Number of Frames; 1 when the attribute is absent or the
        frame model was not read. */
    unsigned int frameCount = 1;

    /** Number of items in (5200,9230); 0 when the sequence is absent. */
    unsigned int perFrameItemCount = 0;

    /** The Pixel Value Transformation of (5200,9229)[0], when it has one. */
    std::optional<Rescale> sharedRescale;

    /** Indexed by per-frame item and sized to perFrameItemCount, which differs
        from frameCount for a file without a frame model. Empty when no
        per-frame item carries a Pixel Value Transformation at all. */
    std::vector<std::optional<Rescale>> perFrameRescale;

    /**
     * \brief Whether every frame of the file has a per-frame functional-group item.
     *
     * A sequence that is present but does not describe every frame cannot be
     * mapped to image slots, so such a file keeps the one-frame model.
     */
    bool HasFrameModel() const
    {
      return this->perFrameItemCount > 0 && this->perFrameItemCount == this->frameCount;
    }
  };

  /** \brief Whether two Pixel Value Transformation pairs are numerically equal. */
  MITKDICOM_EXPORT bool SameRescale(const DICOMFrameLayout::Rescale& left,
                                    const DICOMFrameLayout::Rescale& right);

  /**
   * \brief The Pixel Value Transformation in force for each frame.
   *
   * The frame's own pair where it has one, the shared pair otherwise, and the
   * identity when the file carries neither. One entry per frame; empty for a
   * layout without frames.
   */
  MITKDICOM_EXPORT std::vector<DICOMFrameLayout::Rescale>
    EffectivePerFrameRescale(const DICOMFrameLayout& layout);

  /**
   * \ingroup DICOMModule
   * \brief Conditions that keep a file from getting the frame model, or that a
   *        consumer should know about when interpreting a multi-frame object.
   *
   * Rendered for the log by the reader and reported as data by
   * DICOMVolumeDiagnostics, so that both say the same thing.
   */
  enum class DICOMFrameModelIssue
  {
    /** (5200,9230) has items, but not one per frame. No frame model. */
    RaggedFunctionalGroups,
    /** More than one frame and no per-frame functional groups at all. */
    NoPerFrameMetadata,
    /** Per-frame Pixel Value Transformation pairs differ between frames. */
    VaryingPerFrameRescale,
    /** A shared and a per-frame Pixel Value Transformation in one file. */
    SharedAndPerFrameRescale
  };

  /**
   * \brief Every DICOMFrameModelIssue, in declaration order.
   *
   * A consumer that has to visit all of them iterates this instead of casting a
   * numeric range, so that adding an enumerator is one edit here beside the
   * switches that translate it rather than a silent omission at the call site.
   */
  MITKDICOM_EXPORT const std::vector<DICOMFrameModelIssue>& AllDICOMFrameModelIssues();

  /** \brief Whether a condition is worth a log line (Warning) or only a report entry (Info). */
  enum class DICOMFrameModelSeverity
  {
    Info,
    Warning
  };

  /**
   * \brief One detected condition, carrying everything a log line or a report
   *        entry needs, so that the message and the reported numbers cannot
   *        drift apart.
   */
  struct DICOMFrameModelFinding
  {
    DICOMFrameModelIssue issue = DICOMFrameModelIssue::NoPerFrameMetadata;
    DICOMFrameModelSeverity severity = DICOMFrameModelSeverity::Info;
    /** The files the condition was found in. */
    std::vector<std::string> files;
    unsigned int frameCount = 0;
    unsigned int perFrameItemCount = 0;
    /** Only for VaryingPerFrameRescale. */
    unsigned int distinctRescalePairs = 0;
  };

  /** \brief Short sentence naming the condition, without file names or counts. */
  MITKDICOM_EXPORT std::string DICOMFrameModelIssueToString(DICOMFrameModelIssue issue);

  /** \brief Machine-readable name, snake_case and stable: the key the diagnostics report uses. */
  MITKDICOM_EXPORT std::string DICOMFrameModelIssueToKey(DICOMFrameModelIssue issue);

  /** \brief Machine-readable severity name, snake_case and stable. */
  MITKDICOM_EXPORT std::string DICOMFrameModelSeverityToKey(DICOMFrameModelSeverity severity);

  /**
   * \brief The findings of one file's layout.
   *
   * Everything a frame model can say about one file. A condition that concerns
   * a whole block travels as an IOVolumeSplitReason instead.
   */
  MITKDICOM_EXPORT std::vector<DICOMFrameModelFinding>
    CollectFrameModelFindings(const DICOMFrameLayout& layout, const std::string& filename);
}

#endif
