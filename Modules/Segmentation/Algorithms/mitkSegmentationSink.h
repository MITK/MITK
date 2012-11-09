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

#ifndef MITK_SEGMENTATION_SINK_H_INCLUDET_WAD
#define MITK_SEGMENTATION_SINK_H_INCLUDET_WAD

#include "mitkNonBlockingAlgorithm.h"
#include "SegmentationExports.h"

namespace mitk
{

class Segmentation_EXPORT SegmentationSink : public NonBlockingAlgorithm
{
  public:

    mitkClassMacro( SegmentationSink, NonBlockingAlgorithm )
    mitkAlgorithmNewMacro( SegmentationSink );

  protected:

    SegmentationSink();  // use smart pointers
    virtual ~SegmentationSink();


    virtual void Initialize(const NonBlockingAlgorithm* other = NULL);
    virtual bool ReadyToRun();

    virtual bool ThreadedUpdateFunction(); // will be called from a thread after calling StartAlgorithm

    void InsertBelowGroupNode(mitk::DataNode* node);
    DataNode* LookForPointerTargetBelowGroupNode(const char* name);
    DataNode* GetGroupNode();

  private:

};

} // namespace

#endif


