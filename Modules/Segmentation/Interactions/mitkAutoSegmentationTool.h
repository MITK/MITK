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

#ifndef mitkAutoSegmentationTool_h_Included
#define mitkAutoSegmentationTool_h_Included

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkOperation.h"
#include "mitkTool.h"

namespace mitk
{

/**
  \brief Superclass for tool that create a new segmentation without user interaction in render windows

  This class is undocumented. Ask the creator ($Author$) to supply useful comments.
*/
class Segmentation_EXPORT AutoSegmentationTool : public Tool
{
  public:

    mitkClassMacro(AutoSegmentationTool, Tool);

  protected:

    /// Operation base class, which holds pointers to a node of the data tree (mitk::DataNode)
    /// and to two data sets (mitk::BaseData) instances
    class opExchangeNodes: public mitk::Operation
    {
    public: opExchangeNodes( mitk::OperationType type,  mitk::DataNode* node,
                                                        mitk::BaseData* oldData,
                                                        mitk::BaseData* newData );
            mitk::DataNode* GetNode() { return m_Node; }
            mitk::BaseData* GetOldData() { return m_OldData; }
            mitk::BaseData* GetNewData() { return m_NewData; }
    private: mitk::DataNode::Pointer m_Node;
             mitk::BaseData::Pointer m_OldData;
             mitk::BaseData::Pointer m_NewData;
    };

    AutoSegmentationTool(); // purposely hidden
    virtual ~AutoSegmentationTool();

    virtual const char* GetGroup() const;

    // operation constant
    static const mitk::OperationType OP_EXCHANGE;

    /// \brief Interface of a mitk::StateMachine (for undo/redo)
    void  ExecuteOperation (mitk::Operation*);

    virtual Image::Pointer Get3DImage(Image::Pointer image, unsigned int timestep);

    void InitializeUndoController();

    void PasteSegmentation( Image* targetImage, Image* sourceImage, int pixelvalue, int timestep );

    template<typename TPixel, unsigned int VImageDimension>
    void ItkPasteSegmentation( itk::Image<TPixel,VImageDimension>* targetImage, const mitk::Image* sourceImage, int pixelvalue );
 };

} // namespace

#endif

