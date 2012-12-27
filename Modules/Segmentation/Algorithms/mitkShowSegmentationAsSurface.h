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

#ifndef MITK_SHOW_SEGMENTATION_AS_SURFACE_H_INCLUDET_WAD
#define MITK_SHOW_SEGMENTATION_AS_SURFACE_H_INCLUDET_WAD

#include "mitkSegmentationSink.h"
#include "SegmentationExports.h"
#include "mitkUIDGenerator.h"
#include "mitkSurface.h"

namespace mitk
{

class Segmentation_EXPORT ShowSegmentationAsSurface : public SegmentationSink
{
  public:

    mitkClassMacro( ShowSegmentationAsSurface, SegmentationSink )
    mitkAlgorithmNewMacro( ShowSegmentationAsSurface );

  protected:

    ShowSegmentationAsSurface();  // use smart pointers
    virtual ~ShowSegmentationAsSurface();

    virtual void Initialize(const NonBlockingAlgorithm* other = NULL);
    virtual bool ReadyToRun();

    virtual bool ThreadedUpdateFunction(); // will be called from a thread after calling StartAlgorithm

    virtual void ThreadedUpdateSuccessful(); // will be called from a thread after calling StartAlgorithm

  private:

    UIDGenerator m_UIDGeneratorSurfaces;

    Surface::Pointer m_Surface;
    DataNode::Pointer m_Node;

    bool m_AddToTree;

};

} // namespace

#endif


