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

#ifndef MITK_SEGMENTATION_SINK_H_INCLUDET_WAD
#define MITK_SEGMENTATION_SINK_H_INCLUDET_WAD

#include "mitkNonBlockingAlgorithm.h"

namespace mitk 
{

class MITKEXT_CORE_EXPORT SegmentationSink : public NonBlockingAlgorithm
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

    void InsertBelowGroupNode(mitk::DataTreeNode* node);
    DataTreeNode* LookForPointerTargetBelowGroupNode(const char* name);
    DataTreeNode* GetGroupNode();

  private:

};

} // namespace

#endif


