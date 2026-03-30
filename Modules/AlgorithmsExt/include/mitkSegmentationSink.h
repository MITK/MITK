/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentationSink_h
#define mitkSegmentationSink_h

#include <mitkNonBlockingAlgorithm.h>
#include <MitkAlgorithmsExtExports.h>

namespace mitk
{
  /**
   * \brief A NonBlockingAlgorithm that places segmentation results into the DataStorage.
   *
   * SegmentationSink serves as a base class for algorithms that produce segmentation
   * results (binary images, surfaces, etc.) and insert them into the DataStorage
   * below a specified group node. Subclasses override ThreadedUpdateFunction() to
   * perform the actual segmentation computation.
   *
   * \sa NonBlockingAlgorithm
   * \sa DataStorage
   */
  class MITKALGORITHMSEXT_EXPORT SegmentationSink : public NonBlockingAlgorithm
  {
  public:
    mitkClassMacro(SegmentationSink, NonBlockingAlgorithm);
    mitkAlgorithmNewMacro(SegmentationSink);

  protected:
    SegmentationSink(); // use smart pointers
    ~SegmentationSink() override;

    void Initialize(const NonBlockingAlgorithm *other = nullptr) override;
    bool ReadyToRun() override;

    bool ThreadedUpdateFunction() override; // will be called from a thread after calling StartAlgorithm

    void InsertBelowGroupNode(mitk::DataNode *node);
    DataNode *LookForPointerTargetBelowGroupNode(const char *name);
    DataNode *GetGroupNode();

  private:
  };

} // namespace

#endif
