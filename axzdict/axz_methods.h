#ifndef __axz_methods__
#define __axz_methods__

/*
 * General control method that applies for all plugins
 */
#define AXZ_MID_TEARDOWN                1 
#define AXZ_GENERAL_MID                 99 

/*
 * Plugin: Secure Https - axzhttps
 * Method range: 100-199
 */
#define AXZ_MID_START_HTTPS             100 
#define AXZ_MID_SEND_HTTPS_REQUEST      AXZ_MID_START_HTTPS
#define AXZ_MID_SEND_HTTPS_REQUEST_SYNC AXZ_MID_START_HTTPS + 1
#define AXZ_MID_UPLOAD_HTTPS_FILE       AXZ_MID_START_HTTPS + 2
#define AXZ_MID_END_HTTPS               199 

/*
 * Plugin: Fact collector - axzfc
 * Method range: 200-299
 */
#define AXZ_MID_START_FACT_COLLECTOR    200
#define AXZ_MID_SETUP_SDK               AXZ_MID_START_FACT_COLLECTOR
#define AXZ_MID_GET_SDK_VERSION         AXZ_MID_START_FACT_COLLECTOR + 1
#define AXZ_MID_INVOKE_GENERAL          AXZ_MID_START_FACT_COLLECTOR + 2
#define AXZ_MID_BUILD_REPORT            AXZ_MID_START_FACT_COLLECTOR + 5
#define AXZ_MID_END_FACT_COLLECTOR      299

/*
 * Plugin: Log submitter - axzls
 * Method range: 300-319
 */
#define AXZ_MID_START_LOG_SUBMITTER      300
#define AXZ_MID_SUBMIT_LOG               AXZ_MID_START_LOG_SUBMITTER
#define AXZ_MID_END_LOG_SUBMITTER        319

/*
 * Plugin: Auto upgrade - axzuc
 * Method range: 320-339
 */
#define AXZ_MID_START_UPDATE_CHECKER     320
#define AXZ_MID_AUTO_UPDATE_AGENT        AXZ_MID_START_UPDATE_CHECKER
#define AXZ_MID_AUTO_UPDATE_OESIS        AXZ_MID_START_UPDATE_CHECKER + 1
#define AXZ_MID_END_UPDATE_CHECKER       339

/*
 * Plugin: info agent desk - axziad
 * Method range: 340-399
 */
#define AXZ_MID_START_INFO_DESK          340
#define AXZ_MID_READY_DESK_SERVER        AXZ_MID_START_INFO_DESK
#define AXZ_MID_SHOW_BALLOON             AXZ_MID_START_INFO_DESK + 1
#define AXZ_MID_START_CRSS_DOMAIN_SERVER AXZ_MID_START_INFO_DESK + 2
#define AXZ_MID_STOP_CRSS_DOMAIN_SERVER  AXZ_MID_START_INFO_DESK + 3
#define AXZ_MID_SET_CORS_WHITELIST       AXZ_MID_START_INFO_DESK + 4
#define AXZ_MID_END_INFO_DESK            399

/*
 * Plugin: Cloud - axzcld
 * Method range: 400 -429
 */
#define AXZ_MID_START_CLOUD              400 
#define AXZ_MID_DO_CLOUD_TASKS           AXZ_MID_START_CLOUD
#define AXZ_MID_GET_CLOUD_ACCOUNT        AXZ_MID_START_CLOUD + 1
#define AXZ_MID_RESPONSE_CLOUD_COMMAND   AXZ_MID_START_CLOUD + 2
#define AXZ_MID_QUERY_CONFIG             AXZ_MID_START_CLOUD + 3
#define AXZ_MID_GET_DEVICE_ID            AXZ_MID_START_CLOUD + 4
#define AXZ_MID_UNREGISTER_DEVICE        AXZ_MID_START_CLOUD + 5   
#define AXZ_MID_GET_REM_LINK             AXZ_MID_START_CLOUD + 6
#define AXZ_MID_REPORT_SOH               AXZ_MID_START_CLOUD + 7
#define AXZ_MID_GET_LICENSE_KEY          AXZ_MID_START_CLOUD + 8
#define AXZ_MID_GET_LAST_SEEN            AXZ_MID_START_CLOUD + 9
#define AXZ_MID_SET_SDP_PROFILE          AXZ_MID_START_CLOUD + 10
#define AXZ_MID_GET_COMPLIANCE_STATUS    AXZ_MID_START_CLOUD + 11
#define AXZ_MID_GET_PRIVACY              AXZ_MID_START_CLOUD + 12
#define AXZ_MID_REPORT_SOH_PER_USER      AXZ_MID_START_CLOUD + 13
#define AXZ_MID_REPORT_SOV_PER_USER      AXZ_MID_START_CLOUD + 14
#define AXZ_MID_REPORT_SOP_PER_USER      AXZ_MID_START_CLOUD + 15
#define AXZ_MID_REPORT_SOS_PER_USER      AXZ_MID_START_CLOUD + 16
#define AXZ_MID_GET_CLD_PROFILE          AXZ_MID_START_CLOUD + 17
#define AXZ_MID_REPORT_SOH_SYNC          AXZ_MID_START_CLOUD + 18
#define AXZ_MID_REPORT_SOV_SYNC          AXZ_MID_START_CLOUD + 19
#define AXZ_MID_REPORT_SOP_SYNC          AXZ_MID_START_CLOUD + 20
#define AXZ_MID_GET_AGENT_TOKEN          AXZ_MID_START_CLOUD + 21
#define AXZ_MID_END_CLOUD                429 

/*
 * Plugin: Features - axzfea
 * Method range: 430 - 449
 */
#define AXZ_MID_START_FEATURES           430 
#define AXZ_MID_RUN_FEATURES             AXZ_MID_START_FEATURES
#define AXZ_MID_END_FEATURES             479 

/*
 * Plugin: Operating System Specified - axzoss
 * Method range: 450 - 499
 */
#define AXZ_MID_START_OSS                   450 
#define AXZ_MID_DETECT_INSTALLED_PACKAGES   AXZ_MID_START_OSS
#define AXZ_MID_QUERY_SERVICE_STATE         AXZ_MID_START_OSS + 1
#define AXZ_MID_END_OSS                     499 

/*
 * Plugin: Custom Functional - axzcus
 * Method range: 500 - 549
 */
#define AXZ_MID_START_CUS                  500 
#define AXZ_MID_CUSTOM_SCRIPT              AXZ_MID_START_CUS
#define AXZ_MID_GET_RESULT_CUSTOM_SCRIPT   AXZ_MID_START_CUS + 1
#define AXZ_MID_CUSTOM_SCRIPT_FORCE_RUN    AXZ_MID_START_CUS + 2
#define AXZ_MID_CUSTOM_SCRIPT_GET_TIMEOUT  AXZ_MID_START_CUS + 3
#define AXZ_MID_END_CUS                    549

/*
 * Plugin: Sdp - axzsdp
 * Method range: 550 - 599
 */
#define AXZ_MID_START_SDP                  550
#define AXZ_MID_RUN_SDP                    AXZ_MID_START_SDP + 1
#define AXZ_MID_GET_SDP_VERSION            AXZ_MID_START_SDP + 2
#define AXZ_MID_END_SDP                    599

/*
 * Plugin: Certificate Checker - axzcert
 * Method range: 600 - 649
 */
#define AXZ_MID_START_CERT                  600
#define AXZ_MID_RUN_CERT_CHECKER            AXZ_MID_START_CERT + 1
#define AXZ_MID_SET_CERT_CHECKER_INTERVAL   AXZ_MID_START_CERT + 2
#define AXZ_MID_END_CERT                    649

/*
 * Plugin: MA Work Flow - axzmawf
 * Method range: 650 - 699
 */
#define AXZ_MID_START_MAWF                  650
#define AXZ_MID_RUN_MAWF                    AXZ_MID_START_MAWF + 1
#define AXZ_MID_FETCH_FLOW                  AXZ_MID_START_MAWF + 2
#define AXZ_MID_GET_WORKFLOW_HASH           AXZ_MID_START_MAWF + 3
#define AXZ_MID_SET_PROCESSED_DATA_HASH     AXZ_MID_START_MAWF + 4
#define AXZ_MID_GET_PROCESSED_DATA_HASH     AXZ_MID_START_MAWF + 5
#define AXZ_MID_END_MAWF                    699

/*
 * Plugin: GUI Linux
 * Method range: 700 - 749
 */
#define AXZ_MID_START_GUI                  700
#define AXZ_MID_RUN_GUI                    AXZ_MID_START_GUI + 1
#define AXZ_MID_STOP_GUI                   AXZ_MID_START_GUI + 2
#define AXZ_MID_END_GUI                    749

#endif
