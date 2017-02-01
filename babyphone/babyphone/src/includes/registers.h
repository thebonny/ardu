/*
 * registers.h
 *
 * Created: 01.02.2017 22:52:42
 *  Author: tmueller
 */ 


#ifndef REGISTERS_H_
#define REGISTERS_H_

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


// PIO registers
#define REG_PIOC_PDR	(*(volatile uint32_t*)0x400E1204U)

// ADC registers
#define REG_ADC_CR		(*(volatile uint32_t*)0x400C0000U)
#define REG_ADC_MR		(*(volatile uint32_t*)0x400C0004U)
#define REG_ADC_CHER	(*(volatile uint32_t*)0x400C0010U)
#define REG_ADC_CGR		(*(volatile uint32_t*)0x400C0048U)
#define REG_ADC_IER		(*(volatile uint32_t*)0x400C0024U)

// PMC registers
#define PMC_PCER0		(*(volatile uint32_t*)0x400E0610U)
#define REG_PMC_PCER1	(*(volatile uint32_t*)0x400E0700U)

// TC0 channel 1 registers (PID controller update)
#define TC0_CHANNEL1_CCR  (*(volatile uint32_t*)0x40080040U)
#define TC0_CHANNEL1_IER  (*(volatile uint32_t*)0x40080064U)
#define TC0_CHANNEL1_CMR  (*(volatile uint32_t*)0x40080044U)
#define TC0_CHANNEL1_RC   (*(volatile uint32_t*)0x4008005CU)
#define TC0_CHANNEL1_SR   (*(volatile uint32_t*)0x40080060U)

// TC control bits
#define TC_CMR_TCCLKS_TIMER_CLOCK1 (0x0u << 0)
#define TC_CMR_WAVE (0x1u << 15)
#define TC_CMR_ACPA_CLEAR (0x2u << 16)
#define TC_CMR_ACPC_SET (0x1u << 18)
#define TC_CMR_CPCTRG (0x1u << 14)
#define TC_SR_CPCS (0x1u << 4) /**< \brief (TC_SR) RC Compare Status */


#endif /* REGISTERS_H_ */