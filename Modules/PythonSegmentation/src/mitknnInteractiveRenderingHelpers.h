/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractiveRenderingHelpers_h
#define mitknnInteractiveRenderingHelpers_h

namespace mitk
{
  class DataNode;
}

namespace mitk::nnInteractive
{
  // Hides the given node in every 3D render window by setting a
  // renderer-specific "visible" = false for each registered 3D render window.
  // The global visibility (used by the 2D render windows) is left untouched.
  // nnInteractive prompts are meaningful only on the 2D plane they were drawn
  // on, so mirroring them in 3D just adds clutter.
  //
  // Setting the per-renderer override once at node creation is sufficient even
  // for nodes whose global visibility is toggled afterwards (the live scribble
  // and lasso feedback nodes do this during a stroke): a renderer-specific
  // property takes precedence over the global one. A null node is a no-op.
  void HideNodeIn3DRenderWindows(DataNode* node);
}

#endif
