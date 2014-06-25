/******************** (C) COPYRIGHT 2012 STMicroelectronics ********************
* File Name          : link_layer.h
* Author             : AMS - HEA&RF BU
* Version            : V1.0.0
* Date               : 19-July-2012
* Description        : Header file for BlueNRG's link layer. It contains
*                      definition of functions for link layer, most of which are
*                      mapped to HCI commands.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#ifndef _LINK_LAYER_H
#define _LINK_LAYER_H

#include <ble_status.h>

/******************************************************************************
 * Types
 *****************************************************************************/

/**
 * advertising policy for filtering (white list related) 
 */
typedef tHalUint8   tAdvPolicy;
#define NO_WHITE_LIST_USE           (0X00)
#define WHITE_LIST_FOR_ONLY_SCAN    (0X01)  
#define WHITE_LIST_FOR_ONLY_CONN    (0X02)
#define WHITE_LIST_FOR_ALL          (0X03) 

/**
 * Bluetooth 48 bit address (in little-endian order).
 */
typedef	tHalUint8	tBDAddr[6];


/** 
 * Bluetooth address type
 */
typedef tHalUint8       tAddrType; 
#define RANDOM_ADDR     (1)
#define PUBLIC_ADDR     (0)

/** 
 * Advertising type
 */
typedef tHalUint8       tAdvType;
/**
 * undirected scannable and connectable 
 */ 
#define ADV_IND         (0x00)

/** 
 * directed non scannable
 */
#define ADV_DIRECT_IND  (0x01)

/**
 * scannable non connectable
 */
#define ADV_SCAN_IND    (0x02)

/**
 * non-connectable and no scan response (used for passive scan)
 */
#define ADV_NONCONN_IND (0x03)


/* 0X04-0XFF RESERVED */


/** 
 * lowest allowed interval value for connectable types(20ms)..multiple of 625us
 */
#define ADV_INTERVAL_LOWEST_CONN    (0X0020)

/** 
 * highest allowed interval value (10.24s)..multiple of 625us.
 */
#define ADV_INTERVAL_HIGHEST        (0X4000)

/** 
 * lowest allowed interval value for non connectable types
 * (100ms)..multiple of 625us.
 */
#define ADV_INTERVAL_LOWEST_NONCONN (0X00a0)

/** 
 * Default value of advertising interval for both min/max values.
 * This will be used if host does not specify any advertising parameters 
 * including advIntervalMax and advIntervalMin
 * value = 1.28 sec (in units of 625 us)
 */
#define ADV_INTERVAL_DEFAULT        (0x0800)

#define ADV_CH_37 0x01
#define ADV_CH_38 0x02
#define ADV_CH_39 0x04



#endif /* _LINK_LAYER_H */