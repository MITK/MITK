/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTubeGraphIO_h
#define mitkTubeGraphIO_h

#include <mitkAbstractFileIO.h>
#include <mitkCustomMimeType.h>
#include <mitkIOMimeTypes.h>

#include "mitkTubeGraph.h"

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

    ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    void Write() override;
    ConfidenceLevel GetWriterConfidenceLevel() const override;

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

  protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    TubeGraphIO *IOClone() const override;
    TubeGraphIO(const TubeGraphIO &other);
    const BoundingBox::Pointer ComputeBoundingBox(TubeGraph::Pointer graph) const;
  };
}

#endif
