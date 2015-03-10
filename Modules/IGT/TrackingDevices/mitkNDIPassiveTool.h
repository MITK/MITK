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


#ifndef MITKNDIPASSIVETOOL_H_HEADER_INCLUDED_
#define MITKNDIPASSIVETOOL_H_HEADER_INCLUDED_

#include <mitkInternalTrackingTool.h>
#include "mitkTrackingTypes.h"

namespace mitk
{
  class NDITrackingDevice;
  /**Documentation
  * \brief Implementation of a passive NDI optical tool
  *
  * implements the TrackingTool interface and has the ability to
  * load an srom file that contains the marker configuration for that tool
  *
  * \ingroup IGT
  */
  class MITKIGT_EXPORT NDIPassiveTool : public InternalTrackingTool
  {
  public:
    friend class NDITrackingDevice;
    /**
    * \brief tracking priority for NDI tracking devices
    */
    enum TrackingPriority
    {
      Static    = 'S',
      Dynamic   = 'D',
      ButtonBox = 'B'
    };

    mitkClassMacro(NDIPassiveTool, InternalTrackingTool);


    virtual bool LoadSROMFile(const char* filename);      ///< load a srom tool description file
    virtual const unsigned char* GetSROMData() const;     ///< get loaded srom file as unsigned char array
    virtual unsigned int GetSROMDataLength() const;       ///< get length of SROMData char array

    itkSetStringMacro(PortHandle);    ///< get port handle under which the tool is registered in the tracking device
    itkGetStringMacro(PortHandle);    ///< set port handle under which the tool is registered in the tracking device
    itkSetMacro(TrackingPriority, TrackingPriority);      ///< set tracking priority that the ndi tracking device should use
    itkGetConstMacro(TrackingPriority, TrackingPriority); ///< get tracking priority that the ndi tracking device should use
    itkSetStringMacro(SerialNumber);  ///< set serial number of the tool
    itkGetStringMacro(SerialNumber);  ///< get serial number of the tool
    itkGetStringMacro(File);          ///< get file from which this tool was loaded

  protected:
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    NDIPassiveTool();
    virtual ~NDIPassiveTool();

    unsigned char* m_SROMData;            ///< content of the srom tool description file
    unsigned int m_SROMDataLength;        ///< length of the  srom tool description file
    TrackingPriority m_TrackingPriority;  ///< priority for this tool
    std::string m_PortHandle;             ///< port handle for this tool
    std::string m_SerialNumber;           ///< serial number for this tool
    std::string m_File;                   ///< the original file from which this tool was loaded
  };
} // namespace mitk
#endif /* MITKNDIPASSIVETOOL_H_HEADER_INCLUDED_ */
