#ifndef __axz_names__
#define __axz_names__

#define AXZ_PLUGIN_PREFIX       L"lib"
#define AXZ_PLUGIN_MNG          L"axzmng"
#ifdef _WIN32
    #define AXZ_LIB_EXT      L".dll"
#elif __linux__
    #define AXZ_LIB_EXT      L".so"
#else // mac
    #define AXZ_LIB_EXT      L".dylib"
#endif

#define AXZ_HTTPS_GET           L"get"
#define AXZ_HTTPS_POST          L"post"
#define AXZ_HTTPS_PUT           L"put"
#define AXZ_HTTPS_OPTIONS       L"options"
#define AXZ_HTTPS_UNKNOWN       L"unknown"


//----------------------------<For ballon notification>--------------------------
//
#define AXZ_NOTIFY_STATUS_INFO        L"info"
#define AXZ_NOTIFY_STATUS_WARNING     L"warning"
#define AXZ_NOTIFY_STATUS_QUESTION    L"question"
#define AXZ_NOTIFY_STATUS_ERROR       L"error"


//----------------------------<For log>--------------------------
//
#ifdef _WIN32
    #define AXZ_LOG_DIR         L"logs"
#else
    //--macOS
    //#define AXZ_LOG_DIR         L"/Users/vietnguyen/Documents/Works/"
    //#define AXZ_LOG_DIR         L"/Users/truongthuongit/Documents/Works/"
    //--linux
    #define AXZ_LOG_DIR         L"/var/log/opswatclient/"
    //#define AXZ_LOG_DIR         L"/home/vietnguyen/Desktop/Agent/log/"
    // #define AXZ_LOG_DIR         L"/home/viet/Documents/tmp/opswat/metaxz/"
#endif
#define AXZ_LOG_ID_CORE             0
#define AXZ_LOG_NAME_CORE           L"core.log"

#define AXZ_LOG_ID_SERVICE          1
#define AXZ_LOG_NAME_SERVICE        L"service.log"

#define AXZ_LOG_ID_UPGRADE          2
#define AXZ_LOG_NAME_UPGRADE        L"upgrade.log"

#define AXZ_LOG_ID_TLAC             3
#define AXZ_LOG_NAME_TLAC           L"tlac.log"

#define AXZ_LOG_ID_APP              4
#define AXZ_LOG_NAME_APP            L"app.log"

#define AXZ_LOG_ID_INSTALLER        5
#define AXZ_LOG_NAME_INSTALLER      L"install.log"

#define AXZ_LOG_ID_UNINSTALLER      6
#define AXZ_LOG_NAME_UNINSTALLER    L"uninstall.log"

#define AXZ_LOG_ID_OESIS             7
#define AXZ_LOG_NAME_OESIS          L"oesis.log"

#define AXZ_LOG_ID_F_LOG               8
#define AXZ_LOG_NAME_F              L"fLog.log"

#define AXZ_LOG_LABEL_INFO  	        L"[info]"
#define AXZ_LOG_LABEL_ERROR	            L"[error]"
#define AXZ_LOG_LABEL_DEBUG	            L"[debug]"
#define AXZ_LOG_LABEL_FULL              L"[full]"


//-----------------------------<For single instance>------------------------------
// up to NAME_MAX-4 (i.e. 251) bytes
#define AXZ_LOCKER_SINGLE_AGENT                 "/opswat.metaxz.agent"
#define AXZ_LOCKER_SINGLE_APP                   "/opswat.metaxz.app"
#define AXZ_LOCKER_SINGLE_UPGRADE_UNINSTALL     "/opswat.metaxz.upgrade-uninstall"


//--------------------------------<For location>-------------------------------
//
#define AXZ_DIR_INSTALL_AGENT                   L"/usr/bin/opswatclient"
//#define AXZ_DIR_INSTALL_AGENT                   L"/home/vietnguyen/Desktop/Agent/bin"
#define AXZ_DIR_EXTRACT_UPDATE                  L"/tmp"
// #define AXZ_DIR_EXTRACT_UPDATE                  L"/home/vietnguyen/Desktop/Agent/temp"
#define AXZ_DIR_BACKUP                          L"/var/tmp/opswatclient"
// #define AXZ_DIR_BACKUP                          L"/home/vietnguyen/Desktop/Agent/temp"

//--------------------------------<For general>-------------------------------
//
#define AXZ_OPSWAT_COPYRIGHT_START              "Copyright \xC2\xAE "
#define AXZ_OPSWAT_COPYRIGHT_START_L            L"Copyright (c) "
#define AXZ_OPSWAT_COPYRIGHT_END                " OPSWAT, Inc. All rights reserved"
#define AXZ_OPSWAT_COPYRIGHT_END_L              L" OPSWAT, Inc. All rights reserved"

//--------------------------------<For local storage>---------------------------------------
//
//-- macOS
//#define AXZ_DATA_STORAGE_FOLDER                 L"/Users/vietnguyen/Documents/Works/"
//-- linux

#define AXZ_DATA_STORAGE_FOLDER                 L"/etc/opswatclient/"
// #define AXZ_DATA_STORAGE_FOLDER                 L"/home/vietnguyen/Desktop/Agent/data/"
#define AXZ_PROFILE_STORAGE_PATH                AXZ_DATA_STORAGE_FOLDER L"profile.dat"
#define AXZ_SHARED_PREFERENCES_STORAGE_PATH     AXZ_DATA_STORAGE_FOLDER L"SharedPreferences.dat"
#define AXZ_POST_PROCESS_STORAGE_PATH           AXZ_DATA_STORAGE_FOLDER L"postprocess.act"

//--------------------------------<For socket buffer>---------------------------------------
// 
#define AXZ_SOCKET_BUFFER_SIZE         4096
#define AXZ_LOCAL_HOST                  "127.0.0.1"

//--------------------------------<For Cross domain server>---------------------------------------
//
#define AXZ_CROSS_DOMAIN_DEFAULT_PORT   11369

//--------------------------------<For Desk server>---------------------------------------
//
#define AXZ_DESK_DEFAULT_PORT           55668

//-------------------------------->For post processing>---------------------------------
#define AXZ_POST_PROCESS_UNREGISTER          1
#define AXZ_POST_PROCESS_DELETE              2
#define AXZ_POST_PROCESS_IDENTITY_USER       3

#endif