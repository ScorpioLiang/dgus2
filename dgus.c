/**
  ******************************************************************************
  * @file       dgus.c
  * @author     liang
  * @version    V1.0
  * @date       2023.12.01
  * @brief      DWIN串口屏驱动代码
  * @license    Copyright (c) 2020-2032
  ******************************************************************************
  * @attention
  * DGUS屏采用异步、全双工串口（UART），串口模式为8n1，
  * 即每个数据传送采用十个位，包括1个起始位，8个数据位，1个停止位。
  * 串口的所有指令或数据都是16进制（HEX）格式。
  * 对于字型（2字节）数据，总是采用高字节先传送（MSB）方式，如0x1234先传送0x12。
  * 数据帧中最大包含249字节数据。
  *
  * 修改说明
  * V1.0 2023.12.01
  * 第一次发布
  * V1.1 2024.03.25
  * 重构
  *
  ******************************************************************************
  */
#include "dgus.h"

/*********************************用户接口*************************************/
#include "main.h"
extern UART_HandleTypeDef huart2;
void DGUS_SerialSend(uint8_t *pdata, size_t Size)
{
    if (HAL_UART_Transmit(&huart2, pdata, Size, HAL_MAX_DELAY) != HAL_OK) {
        /* 发送失败 */
    }
}

void DGUS_SerialReceive(uint8_t *pdata, size_t Size)
{

}

DGUS_Display_t default_dgus = {
    .picid = 0,
    .DGUS_SerialSend = DGUS_SerialSend,
    .DGUS_SerialReceive = DGUS_SerialReceive,
    .DGUS_DelyMs = HAL_Delay
};

/*****************************END 用户接口 END*********************************/

/******************************** 基础功能 ************************************/
/**
  * @brief      拼接数据帧
  * @note       根据数据长度的大小按照右对齐放置
  * @param      int16_t addr：数据地址
  * @param      uint64_t data：数据
  * @param      uint8_t size：数据帧大小
  * @param      uint8_t* tx_data：发送的数据帧
  * @retval     void
  */
static inline void _JoinDataFrames(uint8_t *tx_data, uint16_t addr, void *data, uint8_t size)
{
    tx_data[0] = 0x5A;                  /* 帧头 */
    tx_data[1] = 0xA5;
    tx_data[2] = size - 3;              /* 数据长度 */
    tx_data[3] = 0x82;                  /* 写指令 */
    tx_data[4] = (addr >> 8) & 0xFF;    /* 变量地址 */
    tx_data[5] = addr       & 0xFF;

    for (int i = 0; i < size - 6; i++) {
        tx_data[i + 6] = *((uint8_t *)data + ((size - 7) - i)); /* 小端 MSB*/
        /* tx_data[i + 6] = *((uint8_t *)data + i));              大端 LSB */
    }
}

/**
  * @brief      发送二个字节数据
  * @note
  * @param
  * @retval     void
  */
static inline void _Send2Bytes(DGUS_Display_t *dgus, uint16_t addr, uint16_t data)
{
    uint8_t tx_data[8];
    _JoinDataFrames(tx_data, addr, &data, sizeof(tx_data));
    dgus->DGUS_SerialSend(tx_data, sizeof(tx_data));
}

/**
  * @brief      发送四个字节数据
  * @note
  * @param
  * @retval     void
  */
static inline void _Send4Bytes(DGUS_Display_t *dgus, uint16_t addr, uint32_t data)
{
    uint8_t tx_data[10];
    _JoinDataFrames(tx_data, addr, &data, sizeof(tx_data));
    dgus->DGUS_SerialSend(tx_data, sizeof(tx_data));
}

/**
  * @brief      发送八个字节数据
  * @note
  * @param
  * @retval     void
  */
static inline void _Send8Bytes(DGUS_Display_t *dgus, uint16_t addr, uint64_t data)
{
    uint8_t tx_data[14];
    _JoinDataFrames(tx_data, addr, &data, sizeof(tx_data));
    dgus->DGUS_SerialSend(tx_data, sizeof(tx_data));
}

/**
  * @brief      发送单精度浮点数(4字节，float)
  * @note
  * @param
  * @retval     void
  */
static inline void _SendFloat(DGUS_Display_t *dgus, uint16_t addr, float data)
{
    uint8_t tx_data[10];
    _JoinDataFrames(tx_data, addr, &data, sizeof(tx_data));
    dgus->DGUS_SerialSend(tx_data, sizeof(tx_data));
}

/**
  * @brief      发送双精度浮点数(8字节，double)
  * @note
  * @param
  * @retval     void
  */
static inline void _SendDouble(DGUS_Display_t *dgus, uint16_t addr, double data)
{
    uint8_t tx_data[14];
    _JoinDataFrames(tx_data, addr, &data, sizeof(tx_data));
    dgus->DGUS_SerialSend(tx_data, sizeof(tx_data));
}

/*****************************END 基础功能 END*********************************/

/******************************* 系统变量接口 *********************************/
/**
  * @brief      系统复位 System_Reset 0x04
  * @note       重置T5芯片，数据清0,相当于掉电重启
  * @param      void
  * @retval     void
  */
void DGUS_Reset(DGUS_Display_t *dgus)
{
    /* 定义复位数据帧 */
    uint8_t Reset[] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x04, 0x55, 0xAA, 0x5A, 0xA5};
    dgus->DGUS_SerialSend(Reset, 10);
    dgus->DGUS_DelyMs(3000);
}

/**
  * @brief      切换页面 0x84
  * @note       D3：0x5A 表示启动一次页面处理，CPU 处理完清零。
                D2：处理模式。0x01=页面切换（把图片存储区指定的图片显示到当前背景页面）。
                D1:D0：图片 ID。
  * @param      void
  * @retval     void
  */
void DGUS_PicSet(DGUS_Display_t *dgus, uint16_t picid)
{
    /* 定义数据帧 */
    uint8_t tx_data[] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, ((picid >> 8) & 0xFF), (picid & 0xFF)};
    dgus->picid = picid;
    dgus->DGUS_SerialSend(tx_data, 10);
}
/****************************END 系统变量接口 END******************************/

/*************************变量图标显示 VAR Icon（0x00）************************/
/**
  * @brief      变量图标修改图片
  * @note
  * @param
  * @retval     void
  */
void VarIconSetID(DGUS_Display_t *dgus, uint16_t vp_addr, uint16_t id)
{
    _Send2Bytes(dgus, vp_addr, id);
}
/*********************END 变量图标显示 VAR Icon（0x00）END*********************/

/*******************艺术字变量显示 Artistic variable（0x03）*******************/
/**
  * @brief      艺术字变量修改整数数值
  * @note       2字节
  * @param
  * @retval     void
  */
void ArtVarSetInt16(DGUS_Display_t *dgus, uint16_t vp_addr, int16_t data)
{
    _Send2Bytes(dgus, vp_addr, data);
}

/**
  * @brief      艺术字变量修改字库ICON0
  * @note       通过描述指针修改艺术字变量0对应的ICON_ID
  * @param      uint16_t addr : 描述指针地址
  * @param      uint16_t icon_id : 0对应的ICON_ID
  * @retval     void
  */
void ArtVarSetICON0(DGUS_Display_t *dgus, uint16_t sp_addr, uint16_t icon_id)
{
    /* SP描述指针偏移量 0x03 */
    _Send2Bytes(dgus, sp_addr + 0x03, icon_id);
}
/***************END 艺术字变量显示 Artistic variable（0x03）END****************/

/*********************图标旋转指示显示 Icon Rotation（0x05）*******************/
/**
  * @brief      旋转图标修改角度
  * @note
  * @param
  * @retval     void
  */
void RatIconSetAngle(DGUS_Display_t *dgus, uint16_t vp_addr, uint16_t angle)
{
    _Send2Bytes(dgus, vp_addr, angle);
}
/*****************END 图标旋转指示显示 Icon Rotation（0x05）END****************/

/******************** 数据变量显示 data variables（0x10）**********************/
/**
  * @brief      显示整数数据（2字节）
  * @note
  * @param
  * @retval     void
  */
void DataVarSetInt16(DGUS_Display_t *dgus, uint16_t vp_addr, int16_t data)
{
    _Send2Bytes(dgus, vp_addr, data);
}

/**
  * @brief      显示整数数据（4字节）
  * @note
  * @param
  * @retval     void
  */
void DataVarSetInt32(DGUS_Display_t *dgus, uint16_t vp_addr, int32_t data)
{
    _Send4Bytes(dgus, vp_addr, data);
}

/**
  * @brief      显示整数数据（8字节）
  * @note
  * @param
  * @retval     void
  */
void DataVarSetInt64(DGUS_Display_t *dgus, uint16_t vp_addr, int64_t data)
{
    _Send8Bytes(dgus, vp_addr, data);
}

/**
  * @brief      显示单精度浮点数（4字节）
  * @note
  * @param
  * @retval     void
  */
void DataVarSetFloat(DGUS_Display_t *dgus, uint16_t vp_addr, float data)
{
    _SendFloat(dgus, vp_addr, data);
}
/**
  * @brief      显示双精度浮点数（8字节）
  * @note
  * @param
  * @retval     void
  */
void DataVarSetDouble(DGUS_Display_t *dgus, uint16_t vp_addr, double data)
{
    _SendDouble(dgus, vp_addr, data);
}

/********************END 数据变量显示 data variables（0x10）END****************/

/************************ 文本显示 Text Display（0x11）***********************/
/**
  * @brief      GB2312文本显示
  * @note       最大显示512字节长度
  * @param
  * @retval     void
  */
void TextDisplaySetGB2312(DGUS_Display_t *dgus, uint16_t vp_addr, char *textcode, uint16_t len)
{
    uint8_t  tx_data[6] = {0x5A, 0xA5, len + 3, 0x82,
                           (uint8_t)((vp_addr >> 8) & 0xFF), (uint8_t)(vp_addr & 0xFF)
                          };
    dgus->DGUS_SerialSend(tx_data, sizeof(tx_data));
    dgus->DGUS_SerialSend((uint8_t *)textcode, len);
}

/**
  * @brief      文本显示颜色修改
  * @note
  * @param
  * @retval     void
  */
void TextDisplaySetColor(DGUS_Display_t *dgus, uint16_t sp_addr, uint16_t color)
{
    _Send2Bytes(dgus, sp_addr + 0x03, color);
}
/**
  * @brief      文本显示字体大小修改
  * @note
  * @param
  * @retval     void
  */
void TextDisplaySetFontDots(DGUS_Display_t *dgus, uint16_t sp_addr, uint16_t FontDots)
{
    _Send2Bytes(dgus, sp_addr + 0x0A, FontDots);
}
/*************************END 文本显示 Text Display（0x11）END******************/

/***************************** 蜂鸣器 Buzzer **********************************/
/**
  * @brief      蜂鸣器鸣叫
  * @note       一个单位8ms
  * @param      uint16_t mstime : 鸣叫ms时间
  * @retval     void
  */
void BuzzerSet(DGUS_Display_t *dgus, uint16_t mstime)
{
    _Send2Bytes(dgus, 0x00A0, mstime >> 3);
}
/**************************END 蜂鸣器 Buzzer END*******************************/
