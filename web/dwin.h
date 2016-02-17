#ifndef _D_WIN_H
#define _D_WIN_H
#define MAIN_PAGE 1
#define OFF_PAGE 27
#define LIST_PM25_PAGE 8
#define LIST_SHIDU_PAGE 7
#define LIST_TEMP_PAGE 6
#define LIST_HCHO_PAGE 5
#define LIST_CO2_PAGE 4
#define LIST_CO_PAGE 2
#define LOG_IN_PAGE 30
#define CURVE_PAGE	3
#define TIME_SETTING_PAGE 33
#define STATE_E_E_PAGE	29
#define STATE_E_O_PAGE 28
#define STATE_O_O_PAGE 35
#define STATE_O_E_PAGE 9
//page 1
#define ADDR_RUN_TIME_CO	0x0000
#define ADDR_RUN_TIME_CO2	0x0001
#define ADDR_RUN_TIME_HCHO	0x0002
#define ADDR_RUN_TIME_TEMP	0x0003
#define ADDR_RUN_TIME_SHIDU	0x0004
#define ADDR_RUN_TIME_PM25	0x0005
#define TOUCH_RUN_TIME_CO	0x0006
#define TOUCH_RUN_TIME_CO2	0x0007
#define TOUCH_RUN_TIME_HCHO	0x0008
#define TOUCH_RUN_TIME_TEMP	0x0009
#define TOUCH_RUN_TIME_SHIDU	0x000a
#define TOUCH_RUN_TIME_PM25	0x000b
#define TOUCH_DEVICE_STATE	0x000c
#define TOUCH_PRODUCT_INFO	0x000d
#define TOUCH_SYSTEM_SETTING	0x000e
//page 2
#define TOUCH_CO_LAST_PAGE	0x000f
#define TOUCH_CO_NEXT_PAGE	0x0010
#define TOUCH_CO_REFRESH_PAGE	0x0011
#define TOUCH_CO_LIST_RETURN	0x0012
#define ADDR_CO_LIST_TIME_0	0x0013
#define ADDR_CO_LIST_DATA_0	0x0023
#define ADDR_CO_LIST_TIME_1	0x0027
#define ADDR_CO_LIST_DATA_1	0x0037
#define ADDR_CO_LIST_TIME_2	0x002b
#define ADDR_CO_LIST_DATA_2	0x004b
#define ADDR_CO_LIST_TIME_3	0x004f
#define ADDR_CO_LIST_DATA_3	0x005f
#define ADDR_CO_LIST_TIME_4	0x0063
#define ADDR_CO_LIST_DATA_4	0x0073
#define ADDR_CO_LIST_TIME_5	0x0077
#define ADDR_CO_LIST_DATA_5	0x0087
#define ADDR_CO_LIST_TIME_6	0x008b
#define ADDR_CO_LIST_DATA_6	0x008f
#define ADDR_CO_PAGE_NUM	0x0000
#define ADDR_CO_PAGE_TOTAL	0x0000
//page 3
#define ADDR_CURVE_DATA_5	0x0093
#define ADDR_CURVE_DATA_4	0x0097
#define ADDR_CURVE_DATA_3	0x009b
#define ADDR_CURVE_DATA_2	0x009f
#define ADDR_CURVE_DATA_1	0x00a3
#define ADDR_CURVE_DATE	0x00a7
#define ADDR_CURVE_TIME_5	0x00b1
#define ADDR_CURVE_TIME_4	0x00b6
#define ADDR_CURVE_TIME_3	0x00bb
#define ADDR_CURVE_TIME_2	0x00c0
#define ADDR_CURVE_TIME_1	0x00c5
#define ADDR_CURVE_TYPE	0x00ca
#define TOUCH_CURVE_LIST	0x00cb
#define TOUCH_CURVE_RETURN	0x00cc
//page 4
#define TOUCH_CO2_LAST_PAGE	0x0159
#define TOUCH_CO2_NEXT_PAGE	0x015a
#define TOUCH_CO2_REFRESH_PAGE	0x015b
#define TOUCH_CO2_LIST_RETURN	0x015c
#define ADDR_CO2_LIST_TIME_0	0x00cd
#define ADDR_CO2_LIST_DATA_0	0x00dd
#define ADDR_CO2_LIST_TIME_1	0x00e1
#define ADDR_CO2_LIST_DATA_1	0x00f1
#define ADDR_CO2_LIST_TIME_2	0x00f5
#define ADDR_CO2_LIST_DATA_2	0x0105
#define ADDR_CO2_LIST_TIME_3	0x0109
#define ADDR_CO2_LIST_DATA_3	0x0119
#define ADDR_CO2_LIST_TIME_4	0x011d
#define ADDR_CO2_LIST_DATA_4	0x012d
#define ADDR_CO2_LIST_TIME_5	0x0131
#define ADDR_CO2_LIST_DATA_5	0x0141
#define ADDR_CO2_LIST_TIME_6	0x0145
#define ADDR_CO2_LIST_DATA_6	0x0155
#define ADDR_CO2_PAGE_NUM	0x0000
#define ADDR_CO2_PAGE_TOTAL	0x0000
//page 5
#define TOUCH_HCHO_LAST_PAGE	0x01E9
#define TOUCH_HCHO_NEXT_PAGE	0x01EA
#define TOUCH_HCHO_REFRESH_PAGE	0x01EB
#define TOUCH_HCHO_LIST_RETURN	0x01EC
#define ADDR_HCHO_LIST_TIME_0	0x015D
#define ADDR_HCHO_LIST_DATA_0	0x016D
#define ADDR_HCHO_LIST_TIME_1	0x0171
#define ADDR_HCHO_LIST_DATA_1	0x0181
#define ADDR_HCHO_LIST_TIME_2	0x0185
#define ADDR_HCHO_LIST_DATA_2	0x0195
#define ADDR_HCHO_LIST_TIME_3	0x0199
#define ADDR_HCHO_LIST_DATA_3	0x01A9
#define ADDR_HCHO_LIST_TIME_4	0x01AD
#define ADDR_HCHO_LIST_DATA_4	0x01BD
#define ADDR_HCHO_LIST_TIME_5	0x01C1
#define ADDR_HCHO_LIST_DATA_5	0x01D1
#define ADDR_HCHO_LIST_TIME_6	0x01D5
#define ADDR_HCHO_LIST_DATA_6	0x01E5
#define ADDR_HCHO_PAGE_NUM	0x0000
#define ADDR_HCHO_PAGE_TOTAL	0x0000
//page 6	
#define TOUCH_TEMP_LAST_PAGE	0x0279
#define TOUCH_TEMP_NEXT_PAGE	0x027A
#define TOUCH_TEMP_REFRESH_PAGE	0x027B
#define TOUCH_TEMP_LIST_RETURN	0x027C
#define ADDR_TEMP_LIST_TIME_0	0x01ED
#define ADDR_TEMP_LIST_DATA_0	0x01FD
#define ADDR_TEMP_LIST_TIME_1	0x0201
#define ADDR_TEMP_LIST_DATA_1	0x0211
#define ADDR_TEMP_LIST_TIME_2	0x0215
#define ADDR_TEMP_LIST_DATA_2	0x0225
#define ADDR_TEMP_LIST_TIME_3	0x0229
#define ADDR_TEMP_LIST_DATA_3	0x0239
#define ADDR_TEMP_LIST_TIME_4	0x023D
#define ADDR_TEMP_LIST_DATA_4	0x024D
#define ADDR_TEMP_LIST_TIME_5	0x0251
#define ADDR_TEMP_LIST_DATA_5	0x0261
#define ADDR_TEMP_LIST_TIME_6	0x0265
#define ADDR_TEMP_LIST_DATA_6	0x0275
#define ADDR_TEMP_PAGE_NUM	0x0000
#define ADDR_TEMP_PAGE_TOTAL	0x0000
//page 7
#define TOUCH_SHIDU_LAST_PAGE	0x0309
#define TOUCH_SHIDU_NEXT_PAGE	0x030A
#define TOUCH_SHIDU_REFRESH_PAGE	0x030B
#define TOUCH_SHIDU_LIST_RETURN	0x030C
#define ADDR_SHIDU_LIST_TIME_0	0x027D
#define ADDR_SHIDU_LIST_DATA_0	0x028D
#define ADDR_SHIDU_LIST_TIME_1	0x0291
#define ADDR_SHIDU_LIST_DATA_1	0x02A1
#define ADDR_SHIDU_LIST_TIME_2	0x02A5
#define ADDR_SHIDU_LIST_DATA_2	0x02B5
#define ADDR_SHIDU_LIST_TIME_3	0x02B9
#define ADDR_SHIDU_LIST_DATA_3	0x02C9
#define ADDR_SHIDU_LIST_TIME_4	0x02CD
#define ADDR_SHIDU_LIST_DATA_4	0x02DD
#define ADDR_SHIDU_LIST_TIME_5	0x02E1
#define ADDR_SHIDU_LIST_DATA_5	0x02F1
#define ADDR_SHIDU_LIST_TIME_6	0x02F5
#define ADDR_SHIDU_LIST_DATA_6	0x0305
#define ADDR_SHIDU_PAGE_NUM	0x0000
#define ADDR_SHIDU_PAGE_TOTAL	0x0000
//page 8
#define TOUCH_PM25_LAST_PAGE	0x0499
#define TOUCH_PM25_NEXT_PAGE	0x049A
#define TOUCH_PM25_REFRESH_PAGE	0x049B
#define TOUCH_PM25_LIST_RETURN	0x049C
#define ADDR_PM25_LIST_TIME_0	0x040D
#define ADDR_PM25_LIST_DATA_0	0x041D
#define ADDR_PM25_LIST_TIME_1	0x0421
#define ADDR_PM25_LIST_DATA_1	0x0431
#define ADDR_PM25_LIST_TIME_2	0x0435
#define ADDR_PM25_LIST_DATA_2	0x0445
#define ADDR_PM25_LIST_TIME_3	0x0449
#define ADDR_PM25_LIST_DATA_3	0x0459
#define ADDR_PM25_LIST_TIME_4	0x045D
#define ADDR_PM25_LIST_DATA_4	0x046D
#define ADDR_PM25_LIST_TIME_5	0x0471
#define ADDR_PM25_LIST_DATA_5	0x0481
#define ADDR_PM25_LIST_TIME_6	0x0485
#define ADDR_PM25_LIST_DATA_6	0x0495
#define ADDR_PM25_PAGE_NUM	0x0000
#define ADDR_PM25_PAGE_TOTAL	0x0000
//page 9
#define TOUCH_STATE_RETURN_1	0x049d
//page 10	
#define ADDR_PRODUCT_NAME	0x049e
#define ADDR_PRODUCT_MODE	0x04c6
#define ADDR_PRODUCT_ID	0x04EE
#define TOUCH_PRODUCT_RETURN	0x049e
//page 11
#define TOUCH_COMPANY_RETURN	0x0516
//page 12
#define ADDR_INTERFACE_1	0x0517
#define ADDR_INTERFACE_2	0x0521
#define ADDR_INTERFACE_3	0x052b
#define ADDR_INTERFACE_4	0x0535
#define ADDR_INTERFACE_5	0x053f
#define ADDR_INTERFACE_6	0x0549
#define TOUCH_INTERFACE_OK	0x0553
#define TOUCH_INTERFACE_RETURN	0x0554
#define TOUCH_INTERFACE_1	0x0555
#define TOUCH_INTERFACE_2	0x0556
#define TOUCH_INTERFACE_3	0x0557
#define TOUCH_INTERFACE_4	0x0558
#define TOUCH_INTERFACE_5	0x0559
#define TOUCH_INTERFACE_6	0x055a
//page 13
#define TOUCH_TUN_ZERO	0x055b
#define TOUCH_VERIFY	0x055c
#define TOUCH_INTERFACE_SET	0x055d
#define TOUCH_SETTING_RETURN	0x055e
//page 14
#define TOUCH_SET_HCHO_1	0x055f
#define TOUCH_SET_CO_1	0x0560
#define TOUCH_SET_CO2_1	0x0561
#define TOUCH_SET_TEMP_1	0x0562
#define TOUCH_SET_SHIDU_1	0x0563
#define TOUCH_SET_PM25_1	0x0564
#define TOUCH_SET_HCHO_2	0x0565
#define TOUCH_SET_CO_2	0x0566
#define TOUCH_SET_CO2_2	0x0567
#define TOUCH_SET_TEMP_2	0x0568
#define TOUCH_SET_SHIDU_2	0x0569
#define TOUCH_SET_PM25_2	0x056a
#define TOUCH_SET_HCHO_3	0x056b
#define TOUCH_SET_CO_3	0x056c
#define TOUCH_SET_CO2_3	0x056d
#define TOUCH_SET_TEMP_3	0x056e
#define TOUCH_SET_SHIDU_3	0x056f
#define TOUCH_SET_PM25_3	0x0570
#define TOUCH_SET_RETURN	0x057B
//page 16
#define TOUCH_WIFI_SET_RETURN	0x057c
//page 17
#define ADDR_XFER_MODE	0x057d
#define ADDR_AP_NAME	0x0581
#define ADDR_AP_PASSWD	0x0595
#define TOUCH_SELECT_GPRS	0x05a9
#define TOUCH_SELECT_WIFI	0x05aa
#define TOUCH_SELECT_RETURN	0x05ab
//page 18
#define ADDR_USER_NAME	0x05ac
#define ADDR_INSTALL_PLACE	0x05bc
#define ADDR_USER_INDUSTRY	0x05dc
#define ADDR_USER_ADDR	0x05fc
#define ADDR_USER_PHONE	0x061c
#define ADDR_USER_CONTACTER	0x063c
#define TOUCH_USER_RETURN	0x064c
//page 27
#define TOUCH_SLEEP_WAKE	0x064d
//page 28
#define TOUCH_STATE_RETURN_2	0x064e
//page 29
#define TOUCH_STATE_RETURN_3	0x064f
//page 30
#define ADDR_USER_NAME_VERIFY	0x0650
#define ADDR_USER_PWD_VERIFY	0x0660
#define TOUCH_USER_NAME_VERIFY	0x0650
#define TOUCH_USER_PWD_VERIFY	0x0660
#define TOUCH_USER_OK_VERIFY	0x0670
#define TOUCH_USER_RETURN_VERIFY	0x0671
//page 31
#define TOUCH_SENSOR_SET	0x0672
#define TOUCH_XFER_SET	0x0673
#define TOUCH_TIME_SET	0x0674
#define TOUCH_USER_INFO	0x0675
#define TOUCH_SYS_SET_RETURN	0x0676
//page 33
#define ADDR_TIME_YEAR	0x06a1
#define ADDR_TIME_MONTH	0x06a5
#define ADDR_TIME_DAY	0x06a7
#define ADDR_TIME_HOUR	0x06a9
#define ADDR_TIME_MIN	0x06ab
#define ADDR_TIME_SECOND	0x06ad
#define TOUCH_TIME_CHANGE_MANUL	0x06af
#define TOUCH_TIME_CHANGE_SERVER	0x06b0
#define TOUCH_TIME_CHANGE_RETURN	0x06b1
//page 35
#define TOUCH_STATE_RETURN_4	0x06b2
#endif
