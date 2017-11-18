
#ifndef APP_CONFIG_H
#define APP_CONFIG_H
// <<< Use Configuration Wizard in Context Menu >>>\n
#ifdef USE_APP_CONFIG
#include "app_config.h"
#endif

//==========================================================
// <h> APP logs Module 

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
#define APP_WIFI_CONFIG_LOG_LEVEL 3
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


// </h> 
//==========================================================


#endif //SDK_CONFIG_H

