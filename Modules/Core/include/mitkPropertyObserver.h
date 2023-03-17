/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyObserver_h
#define mitkPropertyObserver_h

#include "MitkCoreExports.h"
#include "mitkCommon.h"
#include <itkEventObject.h>

namespace mitk
{
  /**
    \brief Convenience class to observe changes of a mitk::BaseProperty.

    This class registers itself as an ITK observer to a BaseProperty and gets
    informed of changes to the property. Whenever such a change occurs, the virtual
    method PropertyChanged() or PropertyRemoved() is called. This way, derived
    classes can implement behaviour for more specific properties (e.g. ColorProperty)
    without the need to reimplement the Subject-Observer handling.

  */

  class BaseProperty;

  class MITKCORE_EXPORT PropertyObserver
  {
  public:
    PropertyObserver();
    virtual ~PropertyObserver();

    virtual void PropertyChanged() = 0;
    virtual void PropertyRemoved() = 0;

  protected:
    void BeginModifyProperty();
    void EndModifyProperty();

    unsigned long m_ModifiedTag;
    unsigned long m_DeleteTag;

    bool m_SelfCall;
  };

  class MITKCORE_EXPORT PropertyView : public PropertyObserver
  {
  public:
    PropertyView(const mitk::BaseProperty *);
    ~PropertyView() override;

    void OnModified(const itk::EventObject &e);
    void OnDelete(const itk::EventObject &e);

  protected:
    const mitk::BaseProperty *m_Property;
  };

  class MITKCORE_EXPORT PropertyEditor : public PropertyObserver
  {
  public:
    PropertyEditor(mitk::BaseProperty *);
    ~PropertyEditor() override;

    void OnModified(const itk::EventObject &e);
    void OnDelete(const itk::EventObject &e);

  protected:
    mitk::BaseProperty *m_Property;
  };
}

#endif
