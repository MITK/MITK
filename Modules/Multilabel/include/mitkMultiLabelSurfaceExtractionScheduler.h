/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiLabelSurfaceExtractionScheduler_h
#define mitkMultiLabelSurfaceExtractionScheduler_h

#include <MitkMultilabelExports.h>
#include <mitkLabelSetImage.h>

#include <vtkSmartPointer.h>

#include <memory>
#include <optional>
#include <thread>
#include <vector>

class vtkPolyData;

namespace mitk
{
  /**
   * \brief Extracts the surfaces of multi-label segmentation groups off the thread that owns
   * the data storage.
   *
   * Internal to MultiLabelSegmentationVtkMapper3D, which keeps one per mapper so that all its
   * renderers share the results. Exported only for the tests.
   *
   * Where a thread owns the data storage, as in the Workbench, Request() queues the extraction
   * for a worker and returns at once. Per group, only the latest request waits, and only the
   * last finished extraction is kept. When one finishes, the 3D windows are asked to render
   * again. Without such a thread, as in a command line tool or a test, Request() extracts
   * right away.
   *
   * The worker reads the group image without a lock, unlike other work off the storage thread
   * (see ImageVtkReadView): with a read lock, every brush stroke would wait for the extraction.
   * A stroke during an extraction may therefore leave the result torn. That is accepted because
   * the result is a preview: the stroke has changed the group image MTime, so the result is
   * outdated by its stamp and the next extraction replaces it.
   *
   * vtkSurfaceNets3D cannot be interrupted, so the destructor does not wait for a running
   * extraction. It finishes on its own and is discarded.
   *
   * All methods must be called on the thread that owns the data storage.
   */
  class MITKMULTILABEL_EXPORT MultiLabelSurfaceExtractionScheduler
  {
  public:
    using LabelValueType = MultiLabelSegmentation::LabelValueType;

    /** \brief What a group surface is extracted from. */
    struct MITKMULTILABEL_EXPORT Stamp
    {
      itk::ModifiedTimeType m_DataMTime = 0;
      TimeStepType m_TimeStep = 0;
      bool m_Smoothed = true;

      /**
       * \brief The stamp of extracting the given time step of a group image as it is now.
       * \pre groupImage is not nullptr.
       */
      static Stamp Of(const Image* groupImage, TimeStepType timeStep, bool smoothed);

      bool operator==(const Stamp&) const = default;
    };

    /** \brief A finished extraction. */
    struct Result
    {
      vtkSmartPointer<vtkPolyData> m_Surface;
      Stamp m_Stamp;
    };

    MultiLabelSurfaceExtractionScheduler();
    ~MultiLabelSurfaceExtractionScheduler();

    MultiLabelSurfaceExtractionScheduler(const MultiLabelSurfaceExtractionScheduler&) = delete;
    MultiLabelSurfaceExtractionScheduler& operator=(const MultiLabelSurfaceExtractionScheduler&) = delete;

    /**
     * \brief Ask for the surface of a group image as it is now.
     *
     * Only the latest request per group waits for the worker: this one replaces what is
     * queued, or clears it if an extraction with the same stamp is already running or
     * finished.
     *
     * \param[in] groupImage The group image, which also identifies the group.
     * \param[in] timeStep The time step to extract.
     * \param[in] smoothed Whether to smooth the surface.
     * \param[in] labelValues The labels of the group.
     * \throw mitk::Exception if groupImage is nullptr, does not have the time step, or has a
     * pixel type or dimension without VTK equivalent.
     */
    void Request(const Image* groupImage, TimeStepType timeStep, bool smoothed,
      const std::vector<LabelValueType>& labelValues);

    /** \brief The last finished extraction of a group image, if any. */
    std::optional<Result> GetResult(const Image* groupImage) const;

    /** \brief Whether an extraction of a group image is queued or running. */
    bool IsPending(const Image* groupImage) const;

    /**
     * \brief Drop what is queued for a group image, and its result.
     *
     * A running extraction of it is discarded when it finishes. Call this when a group goes
     * away: its result would otherwise stay alive, and be found for a new image at the same
     * address.
     */
    void Forget(const Image* groupImage);

  private:
    struct Shared;

    std::shared_ptr<Shared> m_Shared;
    std::thread m_Worker;
  };
}

#endif
