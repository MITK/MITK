/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#ifndef MITKNDIPASSIVETOOL_H_HEADER_INCLUDED_
#define MITKNDIPASSIVETOOL_H_HEADER_INCLUDED_

#include <mitkInternalTrackingTool.h>
#include "mitkTrackingTypes.h"

namespace mitk 
{
  //##Documentation
  //## \brief Implementation of a passive NDI optical tool
  //##
  //## implements the TrackingTool interface and has the ability to
  //## load an srom file that contains the marker configuration for that tool
  //## 
  //## @ingroup Tracking
  class NDIPassiveTool : public InternalTrackingTool
  {
  public:
    mitkClassMacro(NDIPassiveTool, InternalTrackingTool);
    itkNewMacro(Self);

    virtual bool LoadSROMFile(const char* filename);      // load a file that contains a srom image file
    virtual const unsigned char* GetSROMData() const;
    virtual unsigned int GetSROMDataLength() const;

    //itkSetMacro(DataValid, bool);
    itkSetStringMacro(PortHandle);
    itkGetStringMacro(PortHandle);
    itkSetMacro(TrackingPriority, TrackingPriority);
    itkGetConstMacro(TrackingPriority, TrackingPriority);
  protected:
    NDIPassiveTool();
    virtual ~NDIPassiveTool();

    unsigned char* m_SROMData;
    unsigned int m_SROMDataLength;
    TrackingPriority m_TrackingPriority;
    std::string m_PortHandle;
  };
} // namespace mitk

#endif /* MITKNDIPASSIVETOOL_H_HEADER_INCLUDED_ */
