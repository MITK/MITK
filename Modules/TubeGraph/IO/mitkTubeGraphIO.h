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

    // TubeGraphIO(TubeGraph::Pointer tubeGraph);
    //????
    TubeGraphIO();


    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;
    virtual std::vector<BaseData::Pointer> Read();

    virtual ConfidenceLevel GetReaderConfidenceLevel() const;

    // -------------- AbstractFileWriter -------------

    virtual void Write();
    virtual ConfidenceLevel GetWriterConfidenceLevel() const;

    static CustomMimeType TUBEGRAPH_MIMETYPE() // tsf
    {
      CustomMimeType mimeType(TUBEGRAPH_MIMETYPE_NAME());
      mimeType.AddExtension("tsf");
      mimeType.SetCategory("Tube Graph");
      mimeType.SetComment("MITK Tube Graph Structure File");
      return mimeType;
    }
    static std::string TUBEGRAPH_MIMETYPE_NAME()
    {
      static std::string name = mitk::IOMimeTypes::DEFAULT_BASE_NAME() + ".tubegraphstructurefile";
      return name;
    }

  private:
    TubeGraphIO* IOClone() const;
    TubeGraphIO(const TubeGraphIO& other);
  };

}

#endif //_MITK_SURFACE_VTK_IO_H_
