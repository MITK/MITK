#ifndef MITKOBJECTOBSERVER_H
#define MITKOBJECTOBSERVER_H

class vtkObject;

namespace itk
{
  class Object;
}

namespace mitk
{

  ///
  /// Abstract interface for classes
  /// that want to be informed when an
  /// itk object was modified or deleted
  ///
  struct ObjectObserver
  {
    virtual void OnModified( const itk::Object* object ) = 0;
    virtual void OnDeleted( const itk::Object* object ) = 0;
    virtual void OnModified( const vtkObject* object ) = 0;
    virtual void OnDeleted( const vtkObject* object ) = 0;
  };

}

#endif // MITKOBJECTOBSERVER_H
