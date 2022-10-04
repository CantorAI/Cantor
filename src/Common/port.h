#pragma once

#if (WIN32)
#define SCANF sscanf_s
#define STRCMPNOCASE strnicmp
#else
#include <strings.h>
#include <memory.h>
#include <netinet/in.h>
#define SCANF sscanf
#define STRCMPNOCASE strncasecmp
#endif