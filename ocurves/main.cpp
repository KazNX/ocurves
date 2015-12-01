//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2012
//
#include "ocurvesconfig.h"

#include <QApplication>
#include <QIcon>

#include "ui/ocurvesui.h"

int main2(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QApplication::setWindowIcon(QIcon(":/icons/ocurves.png"));
  OCurvesUI window;
  app.setActiveWindow(&window);
  window.show();
  window.loadWindowsSettings();
  return app.exec();
}

int main(int argc, char *argv[])
{
#ifdef ALLOCTRACK_ENABLE
  // The following line is here to avoid reporting spurious memory leaks when using Qt.
  // The first Qt instantiation initialises various underlying Qt systems, allocating
  // memory as required. These are not cleaned up until very late in exit and are
  // reported as memory leaks by alloctrack if alloctrack is initialised first.
  // We work around the issue by allocating something which uses Qt and immediately
  // releasing it.
  delete new QString;
  AllocTrackSystem _alloctrack;
#endif // ALLOCTRACK_ENABLE
  return main2(argc, argv);
}
