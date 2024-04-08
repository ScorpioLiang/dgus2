/**
  ******************************************************************************
  * @file       dgus.h
  * @author     liang
  * @version    V1.0
  * @date       2023.12.01
  * @brief      DWIN串口屏驱动头文件
  * @license    Copyright (c) 2020-2032
  ******************************************************************************
  * @attention
  *
  * 修改说明
  * V1.0 2023.12.01
  * 第一次发布
  * V1.1 2024.03.25
  * 重构
  *
  ******************************************************************************
  */
#ifndef __DGUS_H
#define __DGUS_H

#ifdef __cplusplus
extern "C" {
#endif

/* 头文件 */
#include <stdint.h>
#include <string.h>

/* dgus结构体 */
typedef struct DGUS_Display_t {
    char        *model;
    uint16_t    picid;
    void (*DGUS_SerialSend)(uint8_t *pdata, size_t Size);
    void (*DGUS_SerialReceive)(uint8_t *pdata, size_t Size);
    void (*DGUS_DelyMs)(uint32_t ms);
} DGUS_Display_t;

/********************************* 函数声明 ***********************************/

/******************************* 系统变量接口 *********************************/
void DGUS_Reset(DGUS_Display_t *dgus);
void DGUS_PicSet(DGUS_Display_t *dgus, uint16_t picid);

/*************************变量图标显示 VAR Icon（0x00）************************/
void VarIconSetID(DGUS_Display_t *dgus, uint16_t vp_addr, uint16_t id);

/*******************艺术字变量显示 Artistic variable（0x03）*******************/
void ArtVarSetInt16(DGUS_Display_t *dgus, uint16_t vp_addr, int16_t data);
void ArtVarSetICON0(DGUS_Display_t *dgus, uint16_t sp_addr, uint16_t icon_id);

/*********************图标旋转指示显示 Icon Rotation（0x05）*******************/
void RatIconSetAngle(DGUS_Display_t *dgus, uint16_t vp_addr, uint16_t angle);

/******************** 数据变量显示 data variables（0x10）**********************/
void DataVarSetInt16(DGUS_Display_t *dgus, uint16_t vp_addr, int16_t data);
void DataVarSetInt32(DGUS_Display_t *dgus, uint16_t vp_addr, int32_t data);
void DataVarSetInt64(DGUS_Display_t *dgus, uint16_t vp_addr, int64_t data);
void DataVarSetFloat(DGUS_Display_t *dgus, uint16_t vp_addr, float data);
void DataVarSetDouble(DGUS_Display_t *dgus, uint16_t vp_addr, double data);

/************************ 文本显示 Text Display（0x11）***********************/
void TextDisplaySetGB2312(DGUS_Display_t *dgus, uint16_t vp_addr, char *textcode, uint16_t len);
void TextDisplaySetColor(DGUS_Display_t *dgus, uint16_t sp_addr, uint16_t color);
void TextDisplaySetFontDots(DGUS_Display_t *dgus, uint16_t sp_addr, uint16_t FontDots);

/**************************** 蜂鸣器 Buzzer *********************************/
void BuzzerSet(DGUS_Display_t *dgus, uint16_t mstime);

#ifdef __cplusplus
}
#endif

#endif /* __DGUS_H */
