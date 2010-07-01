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
    virtual void OnAdded( const T object ) {}
    virtual void OnModified( const T object ) {}
    ///
    /// Will be called before an object gets removed
    ///
    virtual void OnRemove( const T object ) {}
    virtual void OnDelete( const T object ) {}
    ///
    /// Makes this class abstract
    ///
    virtual ~SetObserver() = 0;
  };

  template <class T>
  SetObserver<T>::~SetObserver() {}

}

#endif // MITKSETOBSERVER_H
