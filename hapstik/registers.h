/*
 * registers.h
 *
 * Created: 01.02.2017 22:52:42
 *  Author: tmueller
 */ 


#ifndef REGISTERS_H_
#define REGISTERS_H_

#include "stdint.h"


// PWM registers
#define REG_PWM_DIS      (*(volatile uint32_t*)0x40094008U) /**< \brief (PWM) PWM Disable Register */
#define REG_PWM_SCM      (*(volatile uint32_t*)0x40094020U) /**< \brief (PWM) PWM Sync Channels Mode Register */
#define REG_PWM_CMR0     (*(volatile uint32_t*)0x40094200U) /**< \brief (PWM) PWM Channel Mode Register (ch_num = 0) */
#define REG_PWM_CMR1     (*(volatile uint32_t*)0x40094220U) /**< \brief (PWM) PWM Channel Mode Register (ch_num = 1) */
#define REG_PWM_CMR2     (*(volatile uint32_t*)0x40094240U) /**< \brief (PWM) PWM Channel Mode Register (ch_num = 2) */
#define REG_PWM_CMR3     (*(volatile uint32_t*)0x40094260U) /**< \brief (PWM) PWM Channel Mode Register (ch_num = 3) */
#define REG_PWM_CMR4     (*(volatile uint32_t*)0x40094280U) /**< \brief (PWM) PWM Channel Mode Register (ch_num = 4) */
#define REG_PWM_CMR5     (*(volatile uint32_t*)0x400942A0U) /**< \brief (PWM) PWM Channel Mode Register (ch_num = 5) */
#define REG_PWM_CMR6     (*(volatile uint32_t*)0x400942C0U) /**< \brief (PWM) PWM Channel Mode Register (ch_num = 6) */

#define REG_PWM_CPRD0    (*(volatile uint32_t*)0x4009420CU) /**< \brief (PWM) PWM Channel Period Register (ch_num = 0) */
#define REG_PWM_CPRD1    (*(volatile uint32_t*)0x4009422CU) /**< \brief (PWM) PWM Channel Period Register (ch_num = 1) */
#define REG_PWM_CPRD2    (*(volatile uint32_t*)0x4009424CU) /**< \brief (PWM) PWM Channel Period Register (ch_num = 2) */
#define REG_PWM_CPRD3    (*(volatile uint32_t*)0x4009426CU) /**< \brief (PWM) PWM Channel Period Register (ch_num = 3) */
#define REG_PWM_CPRD4    (*(volatile uint32_t*)0x4009428CU) /**< \brief (PWM) PWM Channel Period Register (ch_num = 4) */
#define REG_PWM_CPRD5    (*(volatile uint32_t*)0x400942ACU) /**< \brief (PWM) PWM Channel Period Register (ch_num = 5) */
#define REG_PWM_CPRD6    (*(volatile uint32_t*)0x400942CCU) /**< \brief (PWM) PWM Channel Period Register (ch_num = 6) */

#define REG_PWM_CDTY0    (*(volatile uint32_t*)0x40094204U) /**< \brief (PWM) PWM Channel Duty Cycle Register (ch_num = 0) */
#define REG_PWM_CDTY1    (*(volatile uint32_t*)0x40094224U) /**< \brief (PWM) PWM Channel Duty Cycle Register (ch_num = 1) */
#define REG_PWM_CDTY2    (*(volatile uint32_t*)0x40094244U) /**< \brief (PWM) PWM Channel Duty Cycle Register (ch_num = 2) */
#define REG_PWM_CDTY3    (*(volatile uint32_t*)0x40094264U) /**< \brief (PWM) PWM Channel Duty Cycle Register (ch_num = 3) */
#define REG_PWM_CDTY4    (*(volatile uint32_t*)0x40094284U) /**< \brief (PWM) PWM Channel Duty Cycle Register (ch_num = 4) */
#define REG_PWM_CDTY5    (*(volatile uint32_t*)0x400942A4U) /**< \brief (PWM) PWM Channel Duty Cycle Register (ch_num = 5) */
#define REG_PWM_CDTY6    (*(volatile uint32_t*)0x400942C4U) /**< \brief (PWM) PWM Channel Duty Cycle Register (ch_num = 6) */

#define REG_PWM_CDTYUPD0 (*(volatile uint32_t*)0x40094208U) /**< \brief (PWM) PWM Channel Duty Cycle Update Register (ch_num = 0) */
#define REG_PWM_CDTYUPD1 (*(volatile uint32_t*)0x40094228U) /**< \brief (PWM) PWM Channel Duty Cycle Update Register (ch_num = 1) */
#define REG_PWM_CDTYUPD2 (*(volatile uint32_t*)0x40094248U) /**< \brief (PWM) PWM Channel Duty Cycle Update Register (ch_num = 2) */
#define REG_PWM_CDTYUPD3 (*(volatile uint32_t*)0x40094268U) /**< \brief (PWM) PWM Channel Duty Cycle Update Register (ch_num = 3) */
#define REG_PWM_CDTYUPD4 (*(volatile uint32_t*)0x40094288U) /**< \brief (PWM) PWM Channel Duty Cycle Update Register (ch_num = 4) */
#define REG_PWM_CDTYUPD5 (*(volatile uint32_t*)0x400942A8U) /**< \brief (PWM) PWM Channel Duty Cycle Update Register (ch_num = 5) */

#define REG_PWM_CMPM0    (*(volatile uint32_t*)0x40094138U) /**< \brief (PWM) PWM Comparison 0 Mode Register */
#define REG_PWM_CMPV0    (*(volatile uint32_t*)0x40094130U) /**< \brief (PWM) PWM Comparison 0 Value Register */
#define REG_PWM_ELMR     (*(volatile uint32_t*)0x4009407CU) /**< \brief (PWM) PWM Event Line 0 Mode Register */
#define REG_PIOC_PDR     (*(volatile uint32_t*)0x400E1204U) /**< \brief (PIOC) PIO Disable Register */
#define REG_PIOC_ABSR    (*(volatile uint32_t*)0x400E1270U) /**< \brief (PIOC) Peripheral AB Select Register */
#define REG_PWM_ENA      (*(volatile uint32_t*)0x40094004U) /**< \brief (PWM) PWM Enable Register */
#define REG_PWM_SCUC     (*(volatile uint32_t*)0x40094028U) /**< \brief (PWM) PWM Sync Channels Update Control Register */


//	ADC registers
#define REG_ADC_CDR6	(*(volatile  uint32_t*)0x400C0068U) // ADC Channel Data Register
#define REG_ADC_CDR7	(*(volatile  uint32_t*)0x400C006CU) // ADC Channel Data Register

// NVIC registers
#define ISER0  (*(volatile uint32_t*)0xE000E100U)
#define ICER0  (*(volatile uint32_t*)0xE000E180U)
#define ICPR0  (*(volatile uint32_t*)0xE000E280U)
#define ISER1  (*(volatile uint32_t*)0xE000E104U)
#define ICER1  (*(volatile uint32_t*)0xE000E184U)
#define ICPR1  (*(volatile uint32_t*)0xE000E284U)

typedef struct
{
  volatile uint32_t ISER[8];                 /*!< Offset: 0x000 (R/W)  Interrupt Set Enable Register           */
       uint32_t RESERVED0[24];
  volatile uint32_t ICER[8];                 /*!< Offset: 0x080 (R/W)  Interrupt Clear Enable Register         */
       uint32_t RSERVED1[24];
  volatile uint32_t ISPR[8];                 /*!< Offset: 0x100 (R/W)  Interrupt Set Pending Register          */
       uint32_t RESERVED2[24];
  volatile uint32_t ICPR[8];                 /*!< Offset: 0x180 (R/W)  Interrupt Clear Pending Register        */
       uint32_t RESERVED3[24];
  volatile uint32_t IABR[8];                 /*!< Offset: 0x200 (R/W)  Interrupt Active bit Register           */
       uint32_t RESERVED4[56];
  volatile uint8_t  IP[240];                 /*!< Offset: 0x300 (R/W)  Interrupt Priority Register (8Bit wide) */
       uint32_t RESERVED5[644];
  volatile  uint32_t STIR;                    /*!< Offset: 0xE00 ( /W)  Software Trigger Interrupt Register     */
}  NestedIC_Type;

#define SCS_BASE            (0xE000E000UL)                            /*!< System Control Space Base Address  */
#define NVIC_BASE           (SCS_BASE +  0x0100UL)                    /*!< NVIC Base Address                  */
#define __NVIC_PRIO_BITS       4      /**< SAM3X8H uses 4 Bits for the Priority Levels */
#define NestedIC                ((NestedIC_Type      *)     NVIC_BASE     )   /*!< NVIC configuration struct          */


// PIO registers
#define REG_PIOC_PDR	(*(volatile uint32_t*)0x400E1204U)
#define REG_PIOC_SODR   (*(volatile uint32_t*)0x400E1230U) /**< \brief (PIOC) Set Output Data Register */
#define REG_PIOC_CODR   (*(volatile uint32_t*)0x400E1234U) /**< \brief (PIOC) Clear Output Data Register */
#define REG_PIOC_PER    (*(volatile uint32_t*)0x400E1200U) /**< \brief (PIOC) PIO Enable Register */
#define REG_PIOC_OER    (*(volatile uint32_t*)0x400E1210U) /**< \brief (PIOC) Output Enable Register */


#define REG_PIOB_PUDR  (*(volatile uint32_t*)0x400E1060U)
#define REG_PIOB_MDDR  (*(volatile uint32_t*)0x400E1054U)
#define REG_PIOB_IFDR  (*(volatile uint32_t*)0x400E1024U)
#define REG_PIOB_SCIFSR (*(volatile uint32_t*)0x400E1080U)
#define REG_PIOB_ABSR  (*(volatile uint32_t*)0x400E1070U)
#define REG_PIOB_PDR   (*(volatile uint32_t*)0x400E1004U) /**< \brief (PIOB) PIO Disable Register */


// ADC registers
#define REG_ADC_CR		(*(volatile uint32_t*)0x400C0000U)
#define REG_ADC_MR		(*(volatile uint32_t*)0x400C0004U)
#define REG_ADC_CHER	(*(volatile uint32_t*)0x400C0010U)
#define REG_ADC_CGR		(*(volatile uint32_t*)0x400C0048U)
#define REG_ADC_IER		(*(volatile uint32_t*)0x400C0024U)

// PMC registers
#define PMC_PCER0		(*(volatile uint32_t*)0x400E0610U)
#define REG_PMC_PCER1	(*(volatile uint32_t*)0x400E0700U)


// TC0 channel 0 registers (PPM out)
#define TC0_CHANNEL0_CCR  (*(volatile uint32_t*)0x40080000U)
#define TC0_CHANNEL0_IER  (*(volatile uint32_t*)0x40080024U)
#define TC0_CHANNEL0_CMR  (*(volatile uint32_t*)0x40080004U)
#define TC0_CHANNEL0_RC   (*(volatile uint32_t*)0x4008001CU)
#define TC0_CHANNEL0_RA   (*(volatile uint32_t*)0x40080014U)
#define TC0_CHANNEL0_SR   (*(volatile uint32_t*)0x40080020U)

// TC0 channel 1 registers (PID controller update)
#define TC0_CHANNEL1_CCR  (*(volatile uint32_t*)0x40080040U)
#define TC0_CHANNEL1_IER  (*(volatile uint32_t*)0x40080064U)
#define TC0_CHANNEL1_CMR  (*(volatile uint32_t*)0x40080044U)
#define TC0_CHANNEL1_RC   (*(volatile uint32_t*)0x4008005CU)
#define TC0_CHANNEL1_SR   (*(volatile uint32_t*)0x40080060U)

// TC2 channel 0 registers (ppm capture)
#define TC2_CHANNEL0_CCR  (*(volatile uint32_t*)0x40088000U)
#define TC2_CHANNEL0_IER  (*(volatile uint32_t*)0x40088024U)
#define TC2_CHANNEL0_CMR  (*(volatile uint32_t*)0x40088004U)
#define TC2_CHANNEL0_SR   (*(volatile uint32_t*)0x40088020U)
#define TC2_CHANNEL0_RB   (*(volatile uint32_t*)0x40088018U)

// TC2 channel 1 registers (record playback)
#define TC2_CHANNEL1_CCR  (*(volatile uint32_t*)0x40088040U)
#define TC2_CHANNEL1_IER  (*(volatile uint32_t*)0x40088064U)
#define TC2_CHANNEL1_CMR  (*(volatile uint32_t*)0x40088044U)
#define TC2_CHANNEL1_RC   (*(volatile uint32_t*)0x4008805CU)
#define TC2_CHANNEL1_SR   (*(volatile uint32_t*)0x40088060U)

// TC control bits
#define TC_CMR_TCCLKS_TIMER_CLOCK1 (0x0u << 0)
#define TC_CMR_WAVE (0x1u << 15)
#define TC_CMR_ACPA_CLEAR (0x2u << 16)
#define TC_CMR_ACPC_SET (0x1u << 18)
#define TC_CMR_CPCTRG (0x1u << 14)
#define TC_SR_CPCS (0x1u << 4) /**< \brief (TC_SR) RC Compare Status */

#define TC_CMR_TCCLKS_TIMER_CLOCK3 (0x2u << 0) /**< \brief (TC_CMR) Clock selected: TCLK3 */
#define TC_CMR_LDRA_FALLING (0x2u << 16) /**< \brief (TC_CMR) Falling edge of TIOA */
#define TC_CMR_LDRB_RISING (0x1u << 18) /**< \brief (TC_CMR) Rising edge of TIOA */
#define TC_CMR_ABETRG (0x1u << 10) /**< \brief (TC_CMR) TIOA or TIOB External Trigger Selection */
#define TC_CMR_ETRGEDG_RISING (0x1u << 8) /**< \brief (TC_CMR) Rising edge */

// TC interrupt bits
#define TC_IER_LDRBS (0x1u << 6) /**< \brief (TC_IER) RB Loading */
#define TC_SR_LDRBS (0x1u << 6) /**< \brief (TC_SR) RB Loading Status */


#endif /* REGISTERS_H_ */