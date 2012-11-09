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
#ifndef MITKSETOBSERVER_H
#define MITKSETOBSERVER_H

namespace mitk
{
  ///
  /// abstract base class for all objects observing an mitk::Set
  ///
  template <class T>
  struct SetObserver
  {
    virtual void OnAdded( T object ) {}
    virtual void OnModified( T object ) {}
    ///
    /// Will be called before an object gets removed
    ///
    virtual void OnRemove( T object ) {}
    virtual void OnDelete( T object ) {}
    ///
    /// Makes this class abstract
    ///
    virtual ~SetObserver() = 0;
  };

  template <class T>
  SetObserver<T>::~SetObserver() {}

}

#endif // MITKSETOBSERVER_H
