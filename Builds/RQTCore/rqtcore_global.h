#ifndef RQTCORE_GLOBAL_H
#define RQTCORE_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef RQTCORE_LIB
# define RQTCORE_EXPORT Q_DECL_EXPORT
#else
# define RQTCORE_EXPORT Q_DECL_IMPORT
#endif

#endif // RQTCORE_GLOBAL_H
