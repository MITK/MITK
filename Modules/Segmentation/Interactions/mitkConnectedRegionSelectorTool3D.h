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

#ifndef mitkConnectedRegionSelectorTool3D_h_Included
#define mitkConnectedRegionSelectorTool3D_h_Included

#include "SegmentationExports.h"
#include <mitkSegTool3D.h>

namespace mitk
{

class Segmentation_EXPORT ConnectedRegionSelectorTool3D : public SegTool3D
{

public:

   mitkClassMacro(ConnectedRegionSelectorTool3D, SegTool3D);
   itkNewMacro(ConnectedRegionSelectorTool3D);

   /* icon stuff */
   virtual const char** GetXPM() const;
   virtual const char* GetName() const;

   virtual us::ModuleResource GetCursorIconResource() const;
   us::ModuleResource GetIconResource() const;

   virtual void ConnectActionsAndFunctions();

   /// \brief Updates the tool and generates a new preview image
   void Run();

 protected:

   ConnectedRegionSelectorTool3D(); // purposely hidden
   virtual ~ConnectedRegionSelectorTool3D();

   void Deactivated();
   void Activated();

   bool AddRegion (StateMachineAction*, InteractionEvent* interactionEvent);

   template <typename TPixel, unsigned int VImageDimension>
   void InternalAddRegion( itk::Image<TPixel, VImageDimension>* input );

   itk::Index<3> m_LastSeedIndex;
};

} // namespace

#endif
