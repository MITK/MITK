/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNPresets_h
#define QmitkMxNPresets_h

#include <MitkQtWidgetsExports.h>

#include <nlohmann/json.hpp>

#include <QString>

#include <vector>

/**
* \brief The MxN layout presets that ship with MITK, as resources of this
*        module ("mxnLayout_*.json").
*/
namespace QmitkMxNPresets
{
  struct Preset
  {
    /** \brief The resource's file name without its ordering prefix and
    *          extension ("sliceContext" for "mxnLayout_05_sliceContext.json").
    *          It changes neither when a preset is renamed nor when the
    *          presets are reordered. */
    QString id;

    /** \brief The display name, the document's "name". */
    QString name;

    /** \brief The layout document as it ships, for an editor named "mxn". */
    nlohmann::json document;
  };

  /**
  * \brief Every preset, in the order the two-digit prefixes of their file
  *        names define.
  */
  MITKQTWIDGETS_EXPORT const std::vector<Preset>& All();

  /**
  * \brief The document of 'preset' with its window ids qualified for the
  *        editor 'widgetName', ready for QmitkMxNMultiWidget::ApplyLayout.
  */
  MITKQTWIDGETS_EXPORT nlohmann::json DocumentFor(const Preset& preset, const QString& widgetName);
}

#endif
