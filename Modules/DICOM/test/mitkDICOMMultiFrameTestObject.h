/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMMultiFrameTestObject_h
#define mitkDICOMMultiFrameTestObject_h

#include <cstdint>
#include <string>
#include <vector>

namespace mitk
{
  /** \brief One stored frame of a generated multi-frame test object. */
  struct DICOMMultiFrameTestFrame
  {
    /** Fills the whole frame. A constant per frame is what makes "the value at
        slot (t, z) belongs to the frame whose pixels fill that slot" decidable. */
    std::int16_t constant = 0;

    double slope = 1.0;
    double intercept = 0.0;

    /** (0018,9151) Frame Reference DateTime, written when not empty. */
    std::string frameReferenceDateTime;

    /** (0020,9057) In-Stack Position Number and the second Dimension Index Value.
        Independent of the item index, so a test can write them descending while
        the plane positions still ascend. */
    unsigned int inStackPosition = 1;
  };

  /**
   * \brief Recipe for a multi-frame DICOM file written on the fly by a test.
   *
   * MITK-Data holds no object with per-frame functional groups, so the tests of
   * the per-frame read model generate their input. The written file carries only
   * what the MITK read path looks at plus what makes it loadable; it is not a
   * conformant instance of its SOP class.
   */
  struct DICOMMultiFrameTestObject
  {
    unsigned int rows = 8;
    unsigned int columns = 8;

    /** Distance between consecutive plane positions along z, and the pixel
        spacing in both in-plane directions. */
    double sliceSpacing = 4.0;

    /** false: a plain multi-frame object (Nuclear Medicine Image Storage) with
        top-level geometry and rescale and no (5200,92xx) sequence at all. */
    bool functionalGroups = true;

    /** Which functional group carries the Pixel Value Transformation. */
    enum class RescalePlacement
    {
      /** One item per frame, from each frame's slope and intercept. */
      PerFrame,
      /** Only (5200,9229), from the first frame's slope and intercept. */
      Shared,
      /** Per frame as for PerFrame and additionally in (5200,9229) with
          sharedSlopeAlongsidePerFrame, which PS3.3 C.7.6.16 forbids. */
      SharedAndPerFrame
    };
    RescalePlacement rescalePlacement = RescalePlacement::PerFrame;

    /** The shared slope SharedAndPerFrame writes; distinct from every frame's,
        so a test can tell which of the two findings reached a slot. */
    double sharedSlopeAlongsidePerFrame = 88.0;

    /** Write (0020,4000) Image Comments directly into every per-frame item. A
        functional-group item holds only macro sequences on a conformant file, so
        this is the one way to place a single element where only an expansion of
        single-element paths would find it. */
    bool imageCommentsInPerFrameItems = false;

    /** Greater than zero: write that many per-frame items while Number of Frames
        stays at the frame count, i.e. a sequence that does not describe every
        frame. */
    unsigned int perFrameItemCountOverride = 0;

    /** Additionally write the Pixel Value Transformation as a top-level sequence,
        so that a top-level path and a frame-relative path name one attribute. */
    bool duplicateAtTopLevel = false;

    /** The slope duplicateAtTopLevel writes; distinct from every frame's, so a
        test can tell which of the two findings reached a slot. */
    double topLevelDuplicateSlope = 99.0;

    /** Also write Image Position and Orientation (Patient) and Pixel Spacing at
        the top level, as a non-conformant encoder may. A conformant
        functional-group object carries them in the macros only, and the sorters
        read the top level, so only such a file can be sorted into one block
        together with another file. */
    bool topLevelGeometry = false;

    /** Position of the first frame's plane along z. Lets two objects be stacked
        rather than overlapping. */
    double zOffset = 0.0;

    /** Empty: Enhanced PET Image Storage for a functional-group object, Nuclear
        Medicine Image Storage otherwise, with a matching modality. Set both to
        put files of the two kinds into one series, which the sorters would
        otherwise split on SOP Class UID and Modality. */
    std::string sopClassUID;
    std::string modality;

    /** Empty: Write() mints a fresh UID. Setting the same value on two objects
        puts their files into one series, which is what lets the reader sort them
        into one block and makes block separation observable. */
    std::string studyInstanceUID;
    std::string seriesInstanceUID;

    unsigned int instanceNumber = 1;

    std::vector<DICOMMultiFrameTestFrame> frames;

    /** \brief Enhanced PET object of \p frameCount frames.
     *
     * Ascending In-Stack Position, slope 1, intercept 0, and a distinct constant
     * per frame.
     */
    static DICOMMultiFrameTestObject EnhancedPET(unsigned int frameCount);

    /**
     * \brief Write the described file.
     * \return The path of the written file.
     * \throw mitk::Exception if the file cannot be written.
     */
    std::string Write(const std::string& directory, const std::string& filename) const;
  };
}

#endif
