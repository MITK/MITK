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


#ifndef _MITK_SURFACE_STL_IO_H_
#define _MITK_SURFACE_STL_IO_H_

#include "mitkSurfaceVtkIO.h"

namespace mitk
{

class SurfaceStlIO : public mitk::SurfaceVtkIO
{
public:

  SurfaceStlIO();

  // -------------- AbstractFileReader -------------

  using AbstractFileReader::Read;
  virtual std::vector<itk::SmartPointer<BaseData> > Read();

  // -------------- AbstractFileWriter -------------

  virtual void Write();

private:

  SurfaceStlIO* IOClone() const;

  static std::string OPTION_MERGE_POINTS();
  static std::string OPTION_TAG_SOLIDS();
  static std::string OPTION_CLEAN();

};

}

#endif //_MITK_SURFACE_STL_IO_H_
