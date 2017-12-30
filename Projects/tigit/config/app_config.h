
#ifndef APP_CONFIG_H
#define APP_CONFIG_H
// <<< Use Configuration Wizard in Context Menu >>>\n
#ifdef USE_APP_CONFIG
#include "app_config.h"
#endif

//==========================================================
// <h> APP Used Modules
//==========================================================

//==========================================================
// <e> APP_CONF_ENABLE_WINC - Enables the WINC 1500 Module
//==========================================================
#ifndef APP_CONF_ENABLE_WINC
#define APP_CONF_ENABLE_WINC 1
#endif
// <o> APP_SWITCH_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> On

#ifndef APP_USE_WINC
#define APP_USE_WINC 0
#endif

#define APP_USE_WINC_OFF 0
#define APP_USE_WINC_ON 1

// </e>

//==========================================================
// </h>
//==========================================================

//==========================================================
// <h> APP logs Module
//==========================================================

//==========================================================
// <e> APP_RTC_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef APP_RTC_CONFIG_LOG_ENABLED
#define APP_RTC_CONFIG_LOG_ENABLED 1
#endif
// <o> APP_RTC_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef APP_RTC_CONFIG_LOG_LEVEL
#define APP_RTC_CONFIG_LOG_LEVEL 3
#endif

// <o> APP_RTC_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_RTC_CONFIG_INFO_COLOR
#define APP_RTC_CONFIG_INFO_COLOR 0
#endif

// <o> APP_RTC_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_RTC_CONFIG_DEBUG_COLOR
#define APP_RTC_CONFIG_DEBUG_COLOR 0
#endif

// </e>

//==========================================================
// <e> APP_WIFI_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef APP_WIFI_CONFIG_LOG_ENABLED
#define APP_WIFI_CONFIG_LOG_ENABLED 1
#endif
// <o> APP_WIFI_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef APP_WIFI_CONFIG_LOG_LEVEL
#define APP_WIFI_CONFIG_LOG_LEVEL 4
#endif

// <o> APP_WIFI_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_WIFI_CONFIG_INFO_COLOR
#define APP_WIFI_CONFIG_INFO_COLOR 0
#endif

// <o> APP_WIFI_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_WIFI_CONFIG_DEBUG_COLOR
#define APP_WIFI_CONFIG_DEBUG_COLOR 0
#endif

// </e>

//==========================================================
// <e> APP_MQTT_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef APP_MQTT_CONFIG_LOG_ENABLED
#define APP_MQTT_CONFIG_LOG_ENABLED 1
#endif
// <o> APP_MQTT_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef APP_MQTT_CONFIG_LOG_LEVEL
#define APP_MQTT_CONFIG_LOG_LEVEL 4
#endif

// <o> APP_MQTT_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_MQTT_CONFIG_INFO_COLOR
#define APP_MQTT_CONFIG_INFO_COLOR 0
#endif

// <o> APP_MQTT_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_MQTT_CONFIG_DEBUG_COLOR
#define APP_MQTT_CONFIG_DEBUG_COLOR 0
#endif

// </e>


//==========================================================
// <e> APP_SWITCH_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef APP_SWITCH_CONFIG_LOG_ENABLED
#define APP_SWITCH_CONFIG_LOG_ENABLED 1
#endif
// <o> APP_SWITCH_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef APP_SWITCH_CONFIG_LOG_LEVEL
#define APP_SWITCH_CONFIG_LOG_LEVEL 4
#endif

// <o> APP_SWITCH_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_SWITCH_CONFIG_INFO_COLOR
#define APP_SWITCH_CONFIG_INFO_COLOR 0
#endif

// <o> APP_SWITCH_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_SWITCH_CONFIG_DEBUG_COLOR
#define APP_SWITCH_CONFIG_DEBUG_COLOR 0
#endif

// </e>

//==========================================================
// </h>
//==========================================================


#endif //SDK_CONFIG_H

