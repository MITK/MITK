/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef MITK_SHOW_SEGMENTATION_AS_SURFACE_H_INCLUDET_WAD
#define MITK_SHOW_SEGMENTATION_AS_SURFACE_H_INCLUDET_WAD

#include "mitkSegmentationSink.h"
#include "mitkUIDGenerator.h"
#include "mitkSurface.h"

namespace mitk 
{

class MITK_CORE_EXPORT ShowSegmentationAsSurface : public SegmentationSink
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
    DataTreeNode::Pointer m_Node;

    bool m_AddToTree;

};

} // namespace

#endif


