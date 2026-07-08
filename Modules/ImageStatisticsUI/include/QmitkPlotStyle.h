/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPlotStyle_h
#define QmitkPlotStyle_h

/**
 * \brief Light/dark styling for the image-statistics Qwt plots.
 *
 * Chosen by the hosting view from the active Workbench theme and pushed into
 * the histogram and intensity-profile widgets, which have no access to the
 * plugin-level style service themselves.
 */
enum class QmitkPlotStyle
{
  Light,
  Dark
};

#endif
