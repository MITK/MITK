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
