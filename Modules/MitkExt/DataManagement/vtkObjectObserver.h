#ifndef VTKOBJECTOBSERVER_H
#define VTKOBJECTOBSERVER_H

class vtkObject;

class vtkObjectObserver
{
public:
  virtual void OnModified( const vtkObject* object ) = 0;
  virtual void OnDeleted( const vtkObject* object ) = 0;
};

#endif // VTKOBJECTOBSERVER_H
