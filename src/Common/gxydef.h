#ifndef _GXY_DEF_H_
#define _GXY_DEF_H_


typedef void* PasWaitHandle;

#if (WIN32)
#include <Windows.h>
#define Path_Sep_S "\\"
#define Path_Sep '\\'
#define MS_SLEEP(t) Sleep(t)
#define US_SLEEP(t) Sleep(t/1000)
#else
#include <unistd.h>
#include <sys/time.h>
#define Path_Sep_S "/"
#define Path_Sep '/'
#define MS_SLEEP(t)  usleep((t)*1000)
#define US_SLEEP(t) usleep(t)
#endif

enum class DataFrameType
{
    Framework_Type_Begin = 1,
    Framework_ClientRegister,
    Framework_ClientRegister_Ack,
    Framework_Command,
    Framework_Command_Return,
    Framework_RegisterToRegistry,
    Framework_RegisterToRegistry_Ack,
    Framework_GVar,
    Framework_GVar_Ack,
    Framework_Type_End,

    MAX_PASCALID = 10000,
};

#define Cantor_API_Name "cantor"

#endif //_GXY_DEF_H_