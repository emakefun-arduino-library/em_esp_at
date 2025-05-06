#pragma once

#ifndef __CLOGGER_H__
#define __CLOGGER_H__

#define CLOGGER_SEVERITY_VERBOSE (1)
#define CLOGGER_SEVERITY_DEBUG (2)
#define CLOGGER_SEVERITY_INFO (3)
#define CLOGGER_SEVERITY_WARN (4)
#define CLOGGER_SEVERITY_ERROR (5)
#define CLOGGER_SEVERITY_FATAL (6)
#define CLOGGER_SEVERITY_NONE (7)

#ifndef CLOGGER_SEVERITY
#define CLOGGER_SEVERITY CLOGGER_SEVERITY_INFO
#endif

#ifdef ARDUINO_ARCH_ESP32
#include "clogger_esp32.h"
#else
#include "clogger_common.h"
#endif

#endif