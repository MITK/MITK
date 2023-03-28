/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkUSDeviceReaderWriterConstants_h
#define mitkUSDeviceReaderWriterConstants_h

namespace mitk
{
  struct USDeviceReaderWriterConstants
  {
    static const char* TAG_ULTRASOUNDDEVICE;
    static const char* TAG_GENERALSETTINGS;
    static const char* TAG_PROBES;
    static const char* TAG_PROBE;
    static const char* TAG_DEPTHS;
    static const char* TAG_DEPTH;
    static const char* TAG_SPACING;
    static const char* TAG_CROPPING;

    static const char* ATTR_FILEVERS;
    static const char* ATTR_TYPE;
    static const char* ATTR_HOST;
    static const char* ATTR_PORT;
    static const char* ATTR_SERVER;
    static const char* ATTR_NAME;
    static const char* ATTR_MANUFACTURER;
    static const char* ATTR_MODEL;
    static const char* ATTR_COMMENT;
    static const char* ATTR_IMAGESTREAMS;
    static const char* ATTR_GREYSCALE;
    static const char* ATTR_RESOLUTIONOVERRIDE;
    static const char* ATTR_RESOLUTIONWIDTH;
    static const char* ATTR_RESOLUTIONHEIGHT;
    static const char* ATTR_SOURCEID;
    static const char* ATTR_FILEPATH;
    static const char* ATTR_OPENCVPORT;
    static const char* ATTR_DEPTH;
    static const char* ATTR_X;
    static const char* ATTR_Y;
    static const char* ATTR_TOP;
    static const char* ATTR_BOTTOM;
    static const char* ATTR_LEFT;
    static const char* ATTR_RIGHT;
  };
}

#endif
