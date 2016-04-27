#ifndef QMITKHTTPSTATUSCODEHANDLER_H
#define QMITKHTTPSTATUSCODEHANDLER_H

#include <string>

class QmitkHttpStatusCodeHandler final
{

public:
  QmitkHttpStatusCodeHandler();
  ~QmitkHttpStatusCodeHandler();

  bool HandleErrorMessage(const char *_errorMsg);
};

#endif // QMITKHTTPSTATUSCODEHANDLER_H
