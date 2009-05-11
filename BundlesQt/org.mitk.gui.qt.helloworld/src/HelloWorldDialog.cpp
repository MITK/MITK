#include "HelloWorldDialog.h"
#include <QMessageBox>

void HelloWorldDialog::show()
{
  QMessageBox::information(0, "Hello World", "Hello World!");
}

