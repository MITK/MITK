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


#ifndef _MITK_SURFACE_VTK_XML_IO_H_
#define _MITK_SURFACE_VTK_XML_IO_H_

#include "mitkSurfaceVtkIO.h"

#include "mitkBaseData.h"

namespace mitk {

class SurfaceVtkXmlIO : public mitk::SurfaceVtkIO
{
public:

  SurfaceVtkXmlIO();

  // -------------- AbstractFileReader -------------

  using AbstractFileReader::Read;
  virtual std::vector<BaseData::Pointer> Read() override;

  virtual ConfidenceLevel GetReaderConfidenceLevel() const override;

  // -------------- AbstractFileWriter -------------

  virtual void Write() override;

private:

  SurfaceVtkXmlIO* IOClone() const override;
};

}

#endif //_MITK_SURFACE_VTK_XML_IO_H_
