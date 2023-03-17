/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIDataStorageInspectorProvider_h
#define mitkIDataStorageInspectorProvider_h

#include <mitkServiceInterface.h>

#include <MitkQtWidgetsExports.h>

#include <QIcon>

class QmitkAbstractDataStorageInspector;

namespace mitk
{
  /**
   * \ingroup MicroServices_Interfaces
   *
   * \brief The common interface for all DataStorage inspector providers.
   *
   * Implementations of this interface must be registered as a service
   * to make themselves available via the service registry.
   *
   * It is recommended to derive new implementations from QmitkDataStorageInspectorProviderBase
   * which provide correct service registration semantics.
   *
   * \sa QmitkDataStorageInspectorProviderBase
   */
  struct MITKQTWIDGETS_EXPORT IDataStorageInspectorProvider
  {
    virtual ~IDataStorageInspectorProvider();

    /**
     * \brief returns an inspector instance represented by the provider.
     */
    virtual QmitkAbstractDataStorageInspector* CreateInspector() const = 0;

    using InspectorIDType = std::string;
    /** Return the uniqe ID for the inspector type provided.*/
    virtual InspectorIDType GetInspectorID() const = 0;
    /** Return the display name (e.g. used in the UI) for the inspector type provided.*/
    virtual std::string GetInspectorDisplayName() const = 0;
    /** Returns a description of the inspector type provided.*/
    virtual std::string GetInspectorDescription() const = 0;
    /** Returns the svg data of the icon of the inspector. Empty array indicates that no icon is defined.
     @remark It is passed as svg file content and not as icon directly to allow later styling*/
    virtual QIcon GetInspectorIcon() const = 0;

    /**
    * @brief Service property name for the inspector ID.
    *
    * The property value must be of type \c std::string.
    *
    * @return The property name.
    */
    static std::string PROP_INSPECTOR_ID();
  };

} // namespace mitk

MITK_DECLARE_SERVICE_INTERFACE(mitk::IDataStorageInspectorProvider, "org.mitk.IDataStorageInspectorProvider")

#endif
