/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRestApiBridgeBindings_h
#define QmitkRestApiBridgeBindings_h

namespace mitk
{
  class RenderWindowBridge;

  /**
   * \brief Install the Qt/Berry-facing callbacks on a RenderWindowBridge.
   *
   * This is the single entry point from the REST API plugin activator into
   * the Qt-layer code that implements the bridge's provider/getter/setter
   * callbacks (screenshot, selected-position, editor list, StdMulti windows,
   * camera, selected-slice). Keeping this out of the activator itself lets the
   * activator stay focused on Berry/CTK lifecycle and service wiring.
   *
   * The callbacks capture only workbench singletons and render-window lookups
   * by name; no per-call plugin state is retained. All callbacks obey the
   * bridge's typed-exception contract (RenderWindowBridgeNoEditorException /
   * …UnknownWindowException / …UnsupportedOperationException).
   *
   * \param rwb Bridge whose callbacks are to be (re)installed. Not null.
   */
  void ConfigureRestApiBridgeCallbacks(RenderWindowBridge* rwb);
}

#endif
