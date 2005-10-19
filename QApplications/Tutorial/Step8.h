#ifndef STEP8_H
#define STEP8_H

#include "Step6.h"

#ifndef DOXYGEN_IGNORE
class Step8 : public Step6
{
  Q_OBJECT
public:
  Step8( int argc, char* argv[], QWidget *parent=0, const char *name=0 );
  ~Step8() {};

protected:
  virtual void SetupWidgets();
protected slots:
};
#endif // DOXYGEN_IGNORE

#endif // STEP8_H

/**
\example Step8.h
*/
