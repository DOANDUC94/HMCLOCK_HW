/**
 ****************************************************************************************
 *
 * @file user_custs1_def.c
 *
 * @brief Custom Server 1 (CUSTS1) profile database definitions.
 *
 * Copyright (C) 2016-2023 Renesas Electronics Corporation and/or its affiliates.
 * All rights reserved. Confidential Information.
 *
 * This software ("Software") is supplied by Renesas Electronics Corporation and/or its
 * affiliates ("Renesas"). Renesas grants you a personal, non-exclusive, non-transferable,
 * revocable, non-sub-licensable right and license to use the Software, solely if used in
 * or together with Renesas products. You may make copies of this Software, provided this
 * copyright notice and disclaimer ("Notice") is included in all such copies. Renesas
 * reserves the right to change or discontinue the Software at any time without notice.
 *
 * THE SOFTWARE IS PROVIDED "AS IS". RENESAS DISCLAIMS ALL WARRANTIES OF ANY KIND,
 * WHETHER EXPRESS, IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. TO THE
 * MAXIMUM EXTENT PERMITTED UNDER LAW, IN NO EVENT SHALL RENESAS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE, EVEN IF RENESAS HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES. USE OF THIS SOFTWARE MAY BE SUBJECT TO TERMS AND CONDITIONS CONTAINED IN
 * AN ADDITIONAL AGREEMENT BETWEEN YOU AND RENESAS. IN CASE OF CONFLICT BETWEEN THE TERMS
 * OF THIS NOTICE AND ANY SUCH ADDITIONAL LICENSE AGREEMENT, THE TERMS OF THE AGREEMENT
 * SHALL TAKE PRECEDENCE. BY CONTINUING TO USE THIS SOFTWARE, YOU AGREE TO THE TERMS OF
 * THIS NOTICE.IF YOU DO NOT AGREE TO THESE TERMS, YOU ARE NOT PERMITTED TO USE THIS
 * SOFTWARE.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup USER_CONFIG
 * @ingroup USER
 * @brief Custom server 1 (CUSTS1) profile database definitions.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include <stdint.h>
#include "co_utils.h"
#include "prf_types.h"
#include "attm_db_128.h"
#include "user_custs1_def.h"

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

// Service 1 of the custom server 1

static const uint16_t svc1_ctrl_point = 0xff03;
static const uint16_t svc1_adc_val1   = 0xff02;
static const uint16_t svc1_long_value = 0xff01;
#define DEF_SVC1_ADC_VAL_1_USER_DESC         "ADC Value 1"
// Service 1 of the custom server 1
#define DEF_SVC1_UUID_128                {0x59, 0x5a, 0x08, 0xe4, 0x86, 0x2a, 0x9e, 0x8f, 0xe9, 0x11, 0xbc, 0x7c, 0x98, 0x43, 0x42, 0x18}

#define DEF_SVC1_CTRL_POINT_UUID_128     {0x20, 0xEE, 0x8D, 0x0C, 0xE1, 0xF0, 0x4A, 0x0C, 0xB3, 0x25, 0xDC, 0x53, 0x6A, 0x68, 0x86, 0x2D}
#define DEF_SVC1_LONG_VALUE_UUID_128     {0x8C, 0x09, 0xE0, 0xD1, 0x81, 0x54, 0x42, 0x40, 0x8E, 0x4F, 0xD2, 0xB3, 0x77, 0xE3, 0x2A, 0x77}
// Service 1 of the custom server 1
static const att_svc_desc128_t custs1_svc1                      = DEF_SVC1_UUID_128;

static const uint8_t SVC1_CTRL_POINT_UUID_128[ATT_UUID_128_LEN]       = DEF_SVC1_CTRL_POINT_UUID_128;

static const uint8_t SVC1_LONG_VALUE_UUID_128[ATT_UUID_128_LEN]       = DEF_SVC1_LONG_VALUE_UUID_128;

// Attribute specifications
static const uint16_t att_decl_svc       = ATT_DECL_PRIMARY_SERVICE;
static const uint16_t att_decl_char      = ATT_DECL_CHARACTERISTIC;
static const uint16_t att_desc_cfg       = ATT_DESC_CLIENT_CHAR_CFG;
static const uint16_t att_desc_user_desc = ATT_DESC_CHAR_USER_DESCRIPTION;
/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

const uint8_t custs1_services[]  = {SVC1_IDX_SVC, CUSTS1_IDX_NB};
const uint8_t custs1_services_size = ARRAY_LEN(custs1_services) - 1;
const uint16_t custs1_att_max_nb = CUSTS1_IDX_NB;


/// Full CUSTS1 Database Description - Used to add attributes into the database
const struct attm_desc_128 custs1_att_db[CUSTS1_IDX_NB] =
{
    /*************************
     * Service 1 configuration
     *************************
     */

    // Service 1 Declaration
    // Service 1 Declaration服务一定义
    [SVC1_IDX_SVC]                     = {(uint8_t*)&att_decl_svc, ATT_UUID_128_LEN, PERM(RD, ENABLE),
                                            sizeof(custs1_svc1), sizeof(custs1_svc1), (uint8_t*)&custs1_svc1},

//    // Control Point Characteristic Declaration
//    [SVC1_IDX_CONTROL_POINT_CHAR]      = {(uint8_t*)&att_decl_char, ATT_UUID_16_LEN, PERM(RD, ENABLE), 0, 0, NULL},
//    // Control Point Characteristic Value
//    [SVC1_IDX_CONTROL_POINT_VAL]       = {(uint8_t*)&svc1_ctrl_point, ATT_UUID_16_LEN, PERM(WR, ENABLE) | PERM(WRITE_REQ, ENABLE),
//                                            DEF_SVC1_CTRL_POINT_CHAR_LEN, 0, 0},

																		
    // Control Point Characteristic Declaration控制点特征声明 uuid2803
    [SVC1_IDX_CONTROL_POINT_CHAR]      = {(uint8_t*)&att_decl_char, ATT_UUID_16_LEN, PERM(RD, ENABLE),
                                            0, 0, NULL},

    // Control Point Characteristic Value控制点特征值
    [SVC1_IDX_CONTROL_POINT_VAL]       = {SVC1_CTRL_POINT_UUID_128, ATT_UUID_128_LEN, PERM(WR, ENABLE) | PERM(WRITE_REQ, ENABLE),
                                            DEF_SVC1_CTRL_POINT_CHAR_LEN, 0, NULL},

																						
																						
    // ADC Value 1 Characteristic Declaration
    [SVC1_IDX_ADC_VAL_1_CHAR]          = {(uint8_t*)&att_desc_cfg, ATT_UUID_16_LEN, PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(WRITE_REQ, ENABLE),
                                            sizeof(uint16_t), 0, NULL},
    // ADC Value 1 Characteristic Value
    [SVC1_IDX_ADC_VAL_1_VAL]           = {(uint8_t*)&att_desc_user_desc, ATT_UUID_16_LEN, PERM(RD, ENABLE),
                                            sizeof(DEF_SVC1_ADC_VAL_1_USER_DESC) - 1, sizeof(DEF_SVC1_ADC_VAL_1_USER_DESC) - 1,
                                            (uint8_t *) DEF_SVC1_ADC_VAL_1_USER_DESC},

											
//    // Long Value Characteristic Declaration
//    [SVC1_IDX_LONG_VALUE_CHAR]         = {(uint8_t*)&att_decl_char, ATT_UUID_16_LEN, PERM(RD, ENABLE), 0, 0, NULL},
//    // Long Value Characteristic Value
//    [SVC1_IDX_LONG_VALUE_VAL]          = {(uint8_t*)&svc1_long_value, ATT_UUID_16_LEN, PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE) | PERM(WRITE_REQ, ENABLE),
//                                            DEF_SVC1_LONG_VALUE_CHAR_LEN, 0, 0},
		
																						
		    // Long Value Characteristic Declaration
    [SVC1_IDX_LONG_VALUE_CHAR]         = {(uint8_t*)&att_decl_char, ATT_UUID_16_LEN, PERM(RD, ENABLE),
                                            0, 0, NULL},

    // Long Value Characteristic Value
    [SVC1_IDX_LONG_VALUE_VAL]          = {SVC1_LONG_VALUE_UUID_128, ATT_UUID_128_LEN, PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE) | PERM(WRITE_REQ, ENABLE),
                                            DEF_SVC1_LONG_VALUE_CHAR_LEN, 0, NULL},
		

																						
};

/// @} USER_CONFIG
