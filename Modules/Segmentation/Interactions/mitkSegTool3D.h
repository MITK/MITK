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

#ifndef mitkSegTool3D_h_Included
#define mitkSegTool3D_h_Included

#include "mitkTool.h"

//#include "mitkCommon.h"
#include "SegmentationExports.h"
//#include "mitkStateEvent.h"
//#include "mitkPositionEvent.h"
//#include "mitkInteractionConst.h"
#include "mitkToolCommand.h"

namespace mitk
{

/**
  \brief Superclass for tools that operate in 3D

*/
class Segmentation_EXPORT SegTool3D : public Tool
{
  public:

    mitkClassMacro(SegTool3D, Tool);

    /// \brief Stops the running operation and disables the tool.
    void Cancel();

    /// \brief Replaces the active label with the preview image.
    virtual void AcceptPreview();

    /// \brief Replaces the preview image by the difference between the active label and the current preview image.
    virtual void InvertPreview();

    /// \brief Replaces the preview image by the union between the active label and the current preview image.
    virtual void CalculateUnion();

    /// \brief Creates a new label out of the the current preview image.
    virtual void CreateNewLabel(const std::string& name, const mitk::Color& color);

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

    SegTool3D(); // purposely hidden
    SegTool3D(const char*); // purposely hidden
    virtual ~SegTool3D();

    /// \brief Retrieves the name of the group this tool belongs to.
    virtual const char* GetGroup() const;

    // operation constant
    static const mitk::OperationType OP_EXCHANGE;

    /// \brief Interface of a mitk::StateMachine (for undo/redo)
    void  ExecuteOperation (mitk::Operation*);

    virtual void Activated();

    virtual void Deactivated();

    virtual Image::Pointer Get3DImage(Image::Pointer image, unsigned int timestep);

    void InitializeUndoController();

    DataNode::Pointer  m_PreviewNode;
    Image::Pointer     m_PreviewImage;

    ToolCommand::Pointer m_ProgressCommand;

    BaseRenderer*  m_LastEventSender;

    mitk::Point3D m_LastClickedPosition;

    mitk::SlicedData::RegionType m_RequestedRegion;

    template<typename ImageType>
    void InternalAcceptPreview( ImageType* targetImage, const mitk::Image* sourceImage );

    template<typename ImageType>
    void InternalInvertPreview( ImageType* input );

    template<typename ImageType>
    void InternalUnion( ImageType* input );

 };

} // namespace

#endif
