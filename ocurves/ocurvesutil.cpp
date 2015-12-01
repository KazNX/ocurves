//
// @author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "ocurvesutil.h"

#include "ocurvesver.h"

#include <QObject>

namespace ocutil
{
  QString versionString()
  {
    QString verString = QString("%2.%3")
                        .arg(OCURVES_MAJOR_VER).arg(OCURVES_MINOR_VER);

#if OCURVES_PATCH_VER
    verString += QString(".%1").arg(OCURVES_PATCH_VER);
#endif // OCURVES_PATCH_VER

#ifdef OCURVES_BETA_RC
    verString += QString(" %1 %2").arg(OCURVES_BETA_RC).arg(OCURVES_BETA_RC_VER);
#endif // OCURVES_BETA_RC

#if OCURVES_DISPLAY_BUILD
    verString += QString(" %1 %2").arg(QObject::tr("Build")).arg(OCURVES_BUILD_NUMBER);
#endif // OCURVES_DISPLAY_BUILD

    return verString;
  }
}
