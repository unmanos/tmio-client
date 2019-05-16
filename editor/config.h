#ifndef CONFIG_H
#define CONFIG_H

#ifdef QT_DEBUG
#define TMEDITOR_API_SERVER         "http://localhost:990"
#else
//#define API_SERVER      "http://192.168.1.118:990"
//#define TMEDITOR_API_SERVER         "http://ams.servers.tokenmark.io"
#define TMEDITOR_API_SERVER         "https://tokenmark.io"
#endif

#define TMEDITOR_HIDE_WEALTH        false


#endif // CONFIG_H
