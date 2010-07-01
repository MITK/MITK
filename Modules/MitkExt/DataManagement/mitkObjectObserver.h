#ifndef MITKOBJECTOBSERVER_H
#define MITKOBJECTOBSERVER_H

#include <itkObject.h>
#include "mitkSetObserver.h"

namespace mitk
{

  ///
  /// Abstract interface for classes
  /// that want to be informed when an
  /// itk object was modified or deleted
  ///
  struct ObjectObserver: public SetObserver<itk::Object*>
  {
  };
}

#endif // MITKOBJECTOBSERVER_H
