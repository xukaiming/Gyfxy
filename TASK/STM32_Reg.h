/*----------------------------------------------------------------------------
 * Name:    STM32_Reg.h
 * Purpose: STM32Register values and Bit definitions
 * Version: V1.02
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2005-2008 Keil Software. All rights reserved.
 *----------------------------------------------------------------------------
 * History:
 *          V1.02 added register RCC_APB2ENR values
 *          V1.01 added register RCC_CSR values
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/

/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __STM32_REG_H
#define __STM32_REG_H

/*----------------------------------------------------------------------------
   SysTick
 *----------------------------------------------------------------------------*/
/* register SYSTICK_CSR ------------------------------------------------------*/
#define SYSTICK_CSR_ENABLE    ((unsigned long)0x00000001)
#define SYSTICK_CSR_COUNTFLAG ((unsigned long)0x00010000)

/*----------------------------------------------------------------------------*/
///* IRQ Channels --------------------------------------------------------------*/
#define WWDG_IRQChannel              ((u8)0x00)  /* Window WatchDog Interrupt */
#define PVD_IRQChannel               ((u8)0x01)  /* PVD through EXTI Line detection Interrupt */
#define TAMPER_IRQChannel            ((u8)0x02)  /* Tamper Interrupt */
#define RTC_IRQChannel               ((u8)0x03)  /* RTC global Interrupt */
#define FLASH_IRQChannel             ((u8)0x04)  /* FLASH global Interrupt */
#define RCC_IRQChannel               ((u8)0x05)  /* RCC global Interrupt */
#define EXTI0_IRQChannel             ((u8)0x06)  /* EXTI Line0 Interrupt */
#define EXTI1_IRQChannel             ((u8)0x07)  /* EXTI Line1 Interrupt */
#define EXTI2_IRQChannel             ((u8)0x08)  /* EXTI Line2 Interrupt */
#define EXTI3_IRQChannel             ((u8)0x09)  /* EXTI Line3 Interrupt */
#define EXTI4_IRQChannel             ((u8)0x0A)  /* EXTI Line4 Interrupt */
#define DMA1_Channel1_IRQChannel     ((u8)0x0B)  /* DMA1 Channel 1 global Interrupt */
#define DMA1_Channel2_IRQChannel     ((u8)0x0C)  /* DMA1 Channel 2 global Interrupt */
#define DMA1_Channel3_IRQChannel     ((u8)0x0D)  /* DMA1 Channel 3 global Interrupt */
#define DMA1_Channel4_IRQChannel     ((u8)0x0E)  /* DMA1 Channel 4 global Interrupt */
#define DMA1_Channel5_IRQChannel     ((u8)0x0F)  /* DMA1 Channel 5 global Interrupt */
#define DMA1_Channel6_IRQChannel     ((u8)0x10)  /* DMA1 Channel 6 global Interrupt */
#define DMA1_Channel7_IRQChannel     ((u8)0x11)  /* DMA1 Channel 7 global Interrupt */
#define ADC1_2_IRQChannel            ((u8)0x12)  /* ADC1 et ADC2 global Interrupt */
#define USB_HP_CAN_TX_IRQChannel     ((u8)0x13)  /* USB High Priority or CAN TX Interrupts */
#define USB_LP_CAN_RX0_IRQChannel    ((u8)0x14)  /* USB Low Priority or CAN RX0 Interrupts */
#define CAN_RX1_IRQChannel           ((u8)0x15)  /* CAN RX1 Interrupt */
#define CAN_SCE_IRQChannel           ((u8)0x16)  /* CAN SCE Interrupt */
#define EXTI9_5_IRQChannel           ((u8)0x17)  /* External Line[9:5] Interrupts */
#define TIM1_BRK_IRQChannel          ((u8)0x18)  /* TIM1 Break Interrupt */
#define TIM1_UP_IRQChannel           ((u8)0x19)  /* TIM1 Update Interrupt */
#define TIM1_TRG_COM_IRQChannel      ((u8)0x1A)  /* TIM1 Trigger and Commutation Interrupt */
#define TIM1_CC_IRQChannel           ((u8)0x1B)  /* TIM1 Capture Compare Interrupt */
#define TIM2_IRQChannel              ((u8)0x1C)  /* TIM2 global Interrupt */
#define TIM3_IRQChannel              ((u8)0x1D)  /* TIM3 global Interrupt */
#define TIM4_IRQChannel              ((u8)0x1E)  /* TIM4 global Interrupt */
#define I2C1_EV_IRQChannel           ((u8)0x1F)  /* I2C1 Event Interrupt */
#define I2C1_ER_IRQChannel           ((u8)0x20)  /* I2C1 Error Interrupt */
#define I2C2_EV_IRQChannel           ((u8)0x21)  /* I2C2 Event Interrupt */
#define I2C2_ER_IRQChannel           ((u8)0x22)  /* I2C2 Error Interrupt */
#define SPI1_IRQChannel              ((u8)0x23)  /* SPI1 global Interrupt */
#define SPI2_IRQChannel              ((u8)0x24)  /* SPI2 global Interrupt */
#define USART1_IRQChannel            ((u8)0x25)  /* USART1 global Interrupt */
#define USART2_IRQChannel            ((u8)0x26)  /* USART2 global Interrupt */
#define USART3_IRQChannel            ((u8)0x27)  /* USART3 global Interrupt */
#define EXTI15_10_IRQChannel         ((u8)0x28)  /* External Line[15:10] Interrupts */
#define RTCAlarm_IRQChannel          ((u8)0x29)  /* RTC Alarm through EXTI Line Interrupt */
#define USBWakeUp_IRQChannel         ((u8)0x2A)  /* USB WakeUp from suspend through EXTI Line Interrupt */
#define TIM8_BRK_IRQChannel          ((u8)0x2B)  /* TIM8 Break Interrupt */
#define TIM8_UP_IRQChannel           ((u8)0x2C)  /* TIM8 Update Interrupt */
#define TIM8_TRG_COM_IRQChannel      ((u8)0x2D)  /* TIM8 Trigger and Commutation Interrupt */
#define TIM8_CC_IRQChannel           ((u8)0x2E)  /* TIM8 Capture Compare Interrupt */
#define ADC3_IRQChannel              ((u8)0x2F)  /* ADC3 global Interrupt */
#define FSMC_IRQChannel              ((u8)0x30)  /* FSMC global Interrupt */
#define SDIO_IRQChannel              ((u8)0x31)  /* SDIO global Interrupt */
#define TIM5_IRQChannel              ((u8)0x32)  /* TIM5 global Interrupt */
#define SPI3_IRQChannel              ((u8)0x33)  /* SPI3 global Interrupt */
#define UART4_IRQChannel             ((u8)0x34)  /* UART4 global Interrupt */
#define UART5_IRQChannel             ((u8)0x35)  /* UART5 global Interrupt */
#define TIM6_IRQChannel              ((u8)0x36)  /* TIM6 global Interrupt */
#define TIM7_IRQChannel              ((u8)0x37)  /* TIM7 global Interrupt */
#define DMA2_Channel1_IRQChannel     ((u8)0x38)  /* DMA2 Channel 1 global Interrupt */
#define DMA2_Channel2_IRQChannel     ((u8)0x39)  /* DMA2 Channel 2 global Interrupt */
#define DMA2_Channel3_IRQChannel     ((u8)0x3A)  /* DMA2 Channel 3 global Interrupt */
#define DMA2_Channel4_5_IRQChannel   ((u8)0x3B)  /* DMA2 Channel 4 and DMA2 Channel 5 global Interrupt */


#define IS_NVIC_IRQ_CHANNEL(CHANNEL) (((CHANNEL) == WWDG_IRQChannel) || \
                                      ((CHANNEL) == PVD_IRQChannel) || \
                                      ((CHANNEL) == TAMPER_IRQChannel) || \
                                      ((CHANNEL) == RTC_IRQChannel) || \
                                      ((CHANNEL) == FLASH_IRQChannel) || \
                                      ((CHANNEL) == RCC_IRQChannel) || \
                                      ((CHANNEL) == EXTI0_IRQChannel) || \
                                      ((CHANNEL) == EXTI1_IRQChannel) || \
                                      ((CHANNEL) == EXTI2_IRQChannel) || \
                                      ((CHANNEL) == EXTI3_IRQChannel) || \
                                      ((CHANNEL) == EXTI4_IRQChannel) || \
                                      ((CHANNEL) == DMA1_Channel1_IRQChannel) || \
                                      ((CHANNEL) == DMA1_Channel2_IRQChannel) || \
                                      ((CHANNEL) == DMA1_Channel3_IRQChannel) || \
                                      ((CHANNEL) == DMA1_Channel4_IRQChannel) || \
                                      ((CHANNEL) == DMA1_Channel5_IRQChannel) || \
                                      ((CHANNEL) == DMA1_Channel6_IRQChannel) || \
                                      ((CHANNEL) == DMA1_Channel7_IRQChannel) || \
                                      ((CHANNEL) == ADC1_2_IRQChannel) || \
                                      ((CHANNEL) == USB_HP_CAN_TX_IRQChannel) || \
                                      ((CHANNEL) == USB_LP_CAN_RX0_IRQChannel) || \
                                      ((CHANNEL) == CAN_RX1_IRQChannel) || \
                                      ((CHANNEL) == CAN_SCE_IRQChannel) || \
                                      ((CHANNEL) == EXTI9_5_IRQChannel) || \
                                      ((CHANNEL) == TIM1_BRK_IRQChannel) || \
                                      ((CHANNEL) == TIM1_UP_IRQChannel) || \
                                      ((CHANNEL) == TIM1_TRG_COM_IRQChannel) || \
                                      ((CHANNEL) == TIM1_CC_IRQChannel) || \
                                      ((CHANNEL) == TIM2_IRQChannel) || \
                                      ((CHANNEL) == TIM3_IRQChannel) || \
                                      ((CHANNEL) == TIM4_IRQChannel) || \
                                      ((CHANNEL) == I2C1_EV_IRQChannel) || \
                                      ((CHANNEL) == I2C1_ER_IRQChannel) || \
                                      ((CHANNEL) == I2C2_EV_IRQChannel) || \
                                      ((CHANNEL) == I2C2_ER_IRQChannel) || \
                                      ((CHANNEL) == SPI1_IRQChannel) || \
                                      ((CHANNEL) == SPI2_IRQChannel) || \
                                      ((CHANNEL) == USART1_IRQChannel) || \
                                      ((CHANNEL) == USART2_IRQChannel) || \
                                      ((CHANNEL) == USART3_IRQChannel) || \
                                      ((CHANNEL) == EXTI15_10_IRQChannel) || \
                                      ((CHANNEL) == RTCAlarm_IRQChannel) || \
                                      ((CHANNEL) == USBWakeUp_IRQChannel) || \
                                      ((CHANNEL) == TIM8_BRK_IRQChannel) || \
                                      ((CHANNEL) == TIM8_UP_IRQChannel) || \
                                      ((CHANNEL) == TIM8_TRG_COM_IRQChannel) || \
                                      ((CHANNEL) == TIM8_CC_IRQChannel) || \
                                      ((CHANNEL) == ADC3_IRQChannel) || \
                                      ((CHANNEL) == FSMC_IRQChannel) || \
                                      ((CHANNEL) == SDIO_IRQChannel) || \
                                      ((CHANNEL) == TIM5_IRQChannel) || \
                                      ((CHANNEL) == SPI3_IRQChannel) || \
                                      ((CHANNEL) == UART4_IRQChannel) || \
                                      ((CHANNEL) == UART5_IRQChannel) || \
                                      ((CHANNEL) == TIM6_IRQChannel) || \
                                      ((CHANNEL) == TIM7_IRQChannel) || \
                                      ((CHANNEL) == DMA2_Channel1_IRQChannel) || \
                                      ((CHANNEL) == DMA2_Channel2_IRQChannel) || \
                                      ((CHANNEL) == DMA2_Channel3_IRQChannel) || \
                                      ((CHANNEL) == DMA2_Channel4_5_IRQChannel))

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
   PWR
 *----------------------------------------------------------------------------*/
/* register PWR_CR -----------------------------------------------------------*/
//#define PWR_CR_DBP            ((unsigned long)0x00000100)


/*----------------------------------------------------------------------------
   RCC
 *----------------------------------------------------------------------------*/
/* register RCC_CFGR ---------------------------------------------------------*/
//#define RCC_CR_HSION          ((unsigned long)0x00000001)
//#define RCC_CR_HSIRDY         ((unsigned long)0x00000002)
//#define RCC_CR_HSEON          ((unsigned long)0x00010000)
//#define RCC_CR_HSERDY         ((unsigned long)0x00020000)
//#define RCC_CR_PLLON          ((unsigned long)0x01000000)
//#define RCC_CR_PLLRDY         ((unsigned long)0x02000000)

/* register RCC_CFGR ---------------------------------------------------------*/
//#define RCC_CFGR_SW           ((unsigned long)0x00000003)
//#define RCC_CFGR_SWS          ((unsigned long)0x0000000C)
//#define RCC_CFGR_HPRE         ((unsigned long)0x000000F0)
#define RCC_CFGR_PRE1         ((unsigned long)0x00000700)
#define RCC_CFGR_PRE2         ((unsigned long)0x00003800)
//#define RCC_CFGR_PLLSRC       ((unsigned long)0x00010000)
//#define RCC_CFGR_PLLXTPRE     ((unsigned long)0x00020000)
//#define RCC_CFGR_PLLMULL      ((unsigned long)0x003C0000)
//#define RCC_CFGR_USBPRE       ((unsigned long)0x00400000)
//#define RCC_CFGR_MCO          ((unsigned long)0x07000000)

/* register RCC_APB1ENR ------------------------------------------------------*/
//#define RCC_APB1ENR_TIM2EN    ((unsigned long)0x00000001)
//#define RCC_APB1ENR_TIM3EN    ((unsigned long)0x00000002)
//#define RCC_APB1ENR_TIM4EN    ((unsigned long)0x00000004)
//#define RCC_APB1ENR_USART2EN  ((unsigned long)0x00020000)
//#define RCC_APB1ENR_USART3EN  ((unsigned long)0x00040000)
#define RCC_APB1ENR_CANEN     ((unsigned long)0x02000000)
//#define RCC_APB1ENR_BKPEN     ((unsigned long)0x08000000)
//#define RCC_APB1ENR_PWREN     ((unsigned long)0x10000000)

/* register RCC_APB2ENR ------------------------------------------------------*/
//#define RCC_APB2ENR_AFIOEN    ((unsigned long)0x00000001)
//#define RCC_APB2ENR_IOPAEN    ((unsigned long)0x00000004)
//#define RCC_APB2ENR_IOPBEN    ((unsigned long)0x00000008)
//#define RCC_APB2ENR_IOPCEN    ((unsigned long)0x00000010)
//#define RCC_APB2ENR_IOPDEN    ((unsigned long)0x00000020)
//#define RCC_APB2ENR_IOPEEN    ((unsigned long)0x00000040)
//#define RCC_APB2ENR_IOPFEN    ((unsigned long)0x00000080)
//#define RCC_APB2ENR_IOPGEN    ((unsigned long)0x00000100)
//#define RCC_APB2ENR_ADC1EN    ((unsigned long)0x00000200)
//#define RCC_APB2ENR_ADC2EN    ((unsigned long)0x00000400)
//#define RCC_APB2ENR_TIM1EN    ((unsigned long)0x00000800)
//#define RCC_APB2ENR_SPI1EN    ((unsigned long)0x00001000)
//#define RCC_APB2ENR_USART1EN  ((unsigned long)0x00004000)

/* register RCC_BCDR --------------------------------------------------------*/
//#define RCC_BDCR_LSEON        ((unsigned long)0x00000001)
//#define RCC_BDCR_LSERDY       ((unsigned long)0x00000002)
//#define RCC_BDCR_RTCSEL       ((unsigned long)0x00000300)
//#define RCC_BDCR_RTCEN        ((unsigned long)0x00008000)

/* register RCC_CSR ---------------------------------------------------------*/
//#define RCC_CSR_LSION         ((unsigned long)0x00000001)
//#define RCC_CSR_LSIRDY        ((unsigned long)0x00000002)

/*----------------------------------------------------------------------------
   RTC
 *----------------------------------------------------------------------------*/
/* register RTC_CR -----------------------------------------------------------*/
//#define RTC_CRL_SECF          ((unsigned long)0x00000001)
//#define RTC_CRL_ALRF          ((unsigned long)0x00000002)
//#define RTC_CRL_OWF           ((unsigned long)0x00000004)
//#define RTC_CRL_RSF           ((unsigned long)0x00000008)
//#define RTC_CRL_CNF           ((unsigned long)0x00000010)
//#define RTC_CRL_RTOFF         ((unsigned long)0x00000020)


/*----------------------------------------------------------------------------
   TIMX
 *----------------------------------------------------------------------------*/
/* register TIMX_CR1 ---------------------------------------------------------*/
#define TIMX_CR1_CEN         ((unsigned short)0x0001)

/* register TIMX_SR ----------------------------------------------------------*/
#define TIMX_SR_UIF          ((unsigned short)0x0001)


/*----------------------------------------------------------------------------
   CAN
 *----------------------------------------------------------------------------*/
/* register CAN_MCR ----------------------------------------------------------*/
//#define CAN_MCR_INRQ          ((unsigned long)0x00000001)
//#define CAN_MCR_NART          ((unsigned long)0x00000010)

/* register CAN_FMR ----------------------------------------------------------*/
//#define CAN_FMR_FINIT         ((unsigned long)0x00000001)

/* register CAN_TSR ----------------------------------------------------------*/
//#define CAN_TSR_RQCP0         ((unsigned long)0x00000001)
//#define CAN_TSR_TME0          ((unsigned long)0x04000000)

/* register CAN_RF0R ---------------------------------------------------------*/
//#define CAN_RF0R_FMP0         ((unsigned long)0x00000003)
//#define CAN_RF0R_RFOM0        ((unsigned long)0x00000020)

/* register CAN_IER ----------------------------------------------------------*/
//#define CAN_IER_TMEIE         ((unsigned long)0x00000001)
//#define CAN_IER_FMPIE0        ((unsigned long)0x00000002)

/* register CAN_BTR ----------------------------------------------------------*/
//#define CAN_BTR_SILM          ((unsigned long)0x80000000)
//#define CAN_BTR_LBKM          ((unsigned long)0x40000000)

/* register CAN_TIxR ---------------------------------------------------------*/
#define CAN_TIxR_TXRQ         ((unsigned long)0x00000001)

/* register CAN_TDTxR --------------------------------------------------------*/
#define CAN_TDTxR_DLC         ((unsigned long)0x0000000F)

/*----------------------------------------------------------------------------
   ADC
 *----------------------------------------------------------------------------*/
/* register ADC_SR -----------------------------------------------------------*/
//#define ADC_SR_EOC            ((unsigned long)0x00000002)

/* register ADC_DR -----------------------------------------------------------*/
//#define ADC_DR_DATA           ((unsigned long)0x0000FFFF)

/*----------------------------------------------------------------------------
   USART
 *----------------------------------------------------------------------------*/
/* register USART_CR1 --------------------------------------------------------*/
//#define USART_CR1_RE          ((unsigned long)0x00000004)
//#define USART_CR1_TE          ((unsigned long)0x00000008)
//#define USART_CR1_UE          ((unsigned long)0x00002000)

#endif
