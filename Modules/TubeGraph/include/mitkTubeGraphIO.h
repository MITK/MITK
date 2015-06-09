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

#ifndef _MITK_TUBE_GRAPH_IO_H_
#define _MITK_TUBE_GRAPH_IO_H_

#include <mitkAbstractFileIO.h>
#include <mitkCustomMimeType.h>
#include <mitkIOMimeTypes.h>

#include "mitkTubeGraph.h"

class TiXmlElement;

namespace mitk
{

  /**
  * @brief reader and writer for xml representations of mitk::TubeGraph
  *
  * @ingroup IO
  */
  class TubeGraphIO : public mitk::AbstractFileIO
  {
  public:

    TubeGraphIO();

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;
    virtual std::vector<BaseData::Pointer> Read() override;

    virtual ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    virtual void Write() override;
    virtual ConfidenceLevel GetWriterConfidenceLevel() const override;

    static CustomMimeType TUBEGRAPH_MIMETYPE() // tsf
    {
      CustomMimeType mimeType(TUBEGRAPH_MIMETYPE_NAME());
      mimeType.AddExtension("tsf");
      mimeType.SetCategory("Graphs");
      mimeType.SetComment("MITK Tube Graph Structure File");
      return mimeType;
    }
    static std::string TUBEGRAPH_MIMETYPE_NAME()
    {
      static std::string name = mitk::IOMimeTypes::DEFAULT_BASE_NAME() + ".graphs.tubular-sructure";
      return name;
    }

  private:
    TubeGraphIO* IOClone() const override;
    TubeGraphIO(const TubeGraphIO& other);
    const BoundingBox::Pointer ComputeBoundingBox(TubeGraph::Pointer graph ) const;

  };

}

#endif //_MITK_SURFACE_VTK_IO_H_
