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

#ifndef mitkSegmentationsProcessingTool_h_Included
#define mitkSegmentationsProcessingTool_h_Included

#include "mitkCommon.h"
#include <MitkSegmentationExports.h>
#include "mitkTool.h"
#include "mitkDataNode.h"

namespace mitk
{

/**
  \brief Batch processing of all selected segmentations/data

  This class is undocumented. Ask the creator ($Author$) to supply useful comments.
*/
class MITKSEGMENTATION_EXPORT SegmentationsProcessingTool : public Tool
{
  public:

    mitkClassMacro(SegmentationsProcessingTool, Tool);

  protected:

    SegmentationsProcessingTool(); // purposely hidden
    virtual ~SegmentationsProcessingTool();

    virtual const char* GetGroup() const override;

    /**
     \brief Called when the tool gets activated

     Will just call ProcessAllObjects and then deactivate this tool again.
    */
    virtual void Activated() override;
    virtual void Deactivated() override;

    /**
     \brief Loop over all working data items

     Will call the following methods in turn

       - StartProcessingAllData
       - ProcessOneWorkingData(DataNode*) for each node in the working data list of our tool manager
       - FinishProcessingAllData

     Subclasses should override these methods as needed and provide meaningful implementations.
    */
    virtual void ProcessAllObjects();

    /**
      \brief Subclasses should override this method
      \sa ProcessAllObjects
    */
    virtual void StartProcessingAllData();

    /**
      \brief Subclasses should override this method
      \sa ProcessAllObjects
    */
    virtual bool ProcessOneWorkingData( DataNode* node );

    /**
      \brief Subclasses should override this method
      \sa ProcessAllObjects
    */
    virtual void FinishProcessingAllData();

    /**
      \brief Sends an error message if there was an error during processing.
     */
    virtual void SendErrorMessageIfAny();

    /**
      \brief Describes the error (if one occurred during processing).
     */
    virtual std::string GetErrorMessage();

    std::string m_FailedNodes;
 };

} // namespace

#endif

