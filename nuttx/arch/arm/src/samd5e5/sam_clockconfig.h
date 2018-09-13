/************************************************************************************
 * arch/arm/src/samd5e5/sam_clockconfig.h
 *
 *   Copyright (C) 2018 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ************************************************************************************/

#ifndef __ARCH_ARM_SRC_SAMD5E5_SAM_CLOCKCONFIG_H
#define __ARCH_ARM_SRC_SAMD5E5_SAM_CLOCKCONFIG_H

/************************************************************************************
 * Included Files
 ************************************************************************************/

#include <nuttx/config.h>

/************************************************************************************
 * Pre-processor Definitions
 ************************************************************************************/

/************************************************************************************
 * Public Types
 ************************************************************************************/

/* This structure defines the configuration of the 32.768KHz XOSC32 */

struct sam_xosc32_config_s
{
  uint8_t enable     : 1;             /* True:  Enable XOSC32 */
  uint8_t highspeed  : 1;             /* Controls gain of the external oscillator */
  uint8_t extalen    : 1;             /* Crystal oscillator enable
                                       * false:  External clock connected on XIN32
                                       * true:   Crystal connected to XIN32/XOUT32 */
  uint8_t en32k      : 1;             /* Enable 32KHz output */
  uint8_t en1k       : 1;             /* Enable 1KHz output */
  uint8_t runstdby   : 1;             /* Run in standby */
  uint8_t ondemand   : 1;             /* On-demand control */
  uint8_t cfden      : 1;             /* Clock failure detection */
  uint8_t cfdeo      : 1;             /* Clock failure event enable */
  uint8_t caliben    : 1;             /* OSCULP32K calibration enable */
  uint8_t startup;                    /* Start-up time:
                                       *   0:  62592us     4:  2000092us
                                       *   1:  125092us    5:  4000092us
                                       *   2:  500092us    6:  8000092us
                                       *   3:  1000092us */
  uint8_t calib;                      /* OSCULP32K calibration value (0-31) */
  uint8_t rtcsel;                     /* RTC clock selection
                                       *   0 ULP1K 1.024kHz from 32KHz internal ULP
                                       *     oscillator
                                       *   1 ULP32K 32.768kHz from 32KHz internal ULP
                                       *     oscillator
                                       *   4 XOSC1K 1.024kHz from 32KHz external
                                       *     oscillator
                                       *   5 XOSC32K 32.768kHz from 32KHz external crystal
                                       *     oscillator */
};

/* This structure defines the configuration of XOSC0/1 */

struct sam_xosc_config_s
{
  uint8_t enable     : 1;             /* True:  Enable XOSC32 */
  uint8_t extalen    : 1;             /* Crystal oscillator enable
                                       * false:  External clock connected
                                       * true:   Crystal connected */
  uint8_t runstdby   : 1;             /* Run in standby */
  uint8_t ondemand   : 1;             /* On-demand control */
  uint8_t lowgain    : 1;             /* Low buffer gain enable */
  uint8_t enalc      : 1;             /* Automatic loop control enable */
  uint8_t cfden      : 1;             /* Clock failure detection */
  uint8_t swben      : 1;             /* XOSC clock switch enable */
  uint8_t startup;                    /* Start-up time:
                                       *   0  31us      8  7813us
                                       *   1  61us      9  15625us
                                       *   2  122us    10  31250us
                                       *   3  244us    11  62500us
                                       *   4  488us    12  125000us
                                       *   5  977us    13  250000us
                                       *   6  1953us   14  500000us
                                       *   7  3906us   15  1000000us */
  uint32_t xosc_frequency;            /* XOSC frequency */
};

/* This structure defines the configuration of the DFLL0 */

struct sam_dfll_config_s
{
  uint8_t enable     : 1;             /* DFLL enable */
  uint8_t runstdby   : 1;             /* Run in standby */
  uint8_t ondemand   : 1;             /* On-demand control */
  uint8_t mode       : 1;             /* Operating mode selection
                                       *   0  Open-loop operation
                                       *   1  Closed-loop operation */
  uint8_t stable     : 1;             /* Stable DFLL frequency */
  uint8_t llaw       : 1;             /* Lose lock after wake */
  uint8_t usbcrm     : 1;             /* USB clock recovery mode */
  uint8_t ccdis      : 1;             /* Chill cycle disable */
  uint8_t qldis      : 1;             /* Quick Lock Disable */
  uint8_t bplckc     : 1;             /* Bypass coarse clock */
  uint8_t waitlock   : 1;             /* Wait lock */
  uint8_t caliben    : 1;             /* Overwrite factory calibration */
  uint8_t fcalib;                     /* Fine calibration value (if caliben != 0) */
  uint8_t ccalib;                     /* Coarse calibration value (if caliben != 0) */
  uint8_t fstep;                      /* Fine maximum step */
  uint8_t cstep;                      /* Coarse maximum step */
  uint8_t gclk;                       /* GCLK source (if usbcrm != 1 && mode != 0) */
  uint16_t mul;                       /* DFLL multiply factor */
};

/* This structure defines the configuration of the DPLL0/1 */

struct sam_dpll_config_s
{
  uint8_t enable     : 1;             /*  DPLL enable */
  uint8_t dcoen      : 1;             /* DCO filter enable */
  uint8_t lbypass    : 1;             /* Lock bypass */
  uint8_t wuf        : 1;             /* Wake up fast */
  uint8_t runstdby   : 1;             /* Run in standby */
  uint8_t ondemand   : 1;             /* On demand clock activation */
  uint8_t refclk;                     /* Reference clock selection
                                       * 0  Dedicated GCLK clock reference
                                       * 1  XOSC32K clock reference
                                       * 2  XOSC0 clock reference
                                       * 3  XOSC2 clock reference */
  uint8_t ltime;                      /* Lock time
                                       * 0  No time-out. Automatic lock
                                       * 4  Time-out if no locka within 800 us
                                       * 5  Time-out if no locka within 900 us
                                       * 6  Time-out if no locka within 1MS
                                       * 7  Time-out if no locka within 1.1MS */
  uint8_t filter;                     /* Proportional integer filter selection
                                       *     PLL BW    Damping PLL BW    Damping
                                       *  0  92.7 kHz  0.76     8  46.4 kHz  1.49
                                       *  1  131 kHz   1.08     9  65.6 kHz  2.11
                                       *  2  46.4 kHz  0.38    10  23.2 kHz  0.75
                                       *  3  65.6 kHz  0.54    11  32.8 kHz  1.06
                                       *  4  131 kHz   0.56    12  65.6 kHz  1.07
                                       *  5  185 kHz   0.79    13  92.7 kHz  1.51
                                       *  6  65.6 kHz  0.28    14  32.8 kHz  0.53
                                       *  7  92.7 kHz  0.39    15  46.4 kHz  0.75 */
  uint8_t dcofilter;                  /* Sigma-delta DCO filter selection
                                       *
                                       *    Capa pF BW MHz    Capa pF BW MHz
                                       * 0  0.5     3.21   4  2.5     0.64
                                       * 1  1       1.6    5  3       0.55
                                       * 2  1.5     1.1    6  3.5     0.45
                                       * 3  2       0.8    7  4       0.4 */

  uint8_t gclk;                       /* GCLK source (if refclock == 0) */
  uint8_t ldrfrac;                    /* Loop divider fractional part */
  uint16_t ldrint;                    /* Loop divider ratio */
  uint16_t div;                       /* Clock divider */
};

/* This structure defines the configuration of a GCLK */

struct sam_gclk_config_s
{
  uint8_t enable     : 1;             /* True:  Enable GCLK */
  uint8_t idc        : 1;             /* True:  Improve duty cycle */
  uint8_t oov        : 1;             /* True:  Clock output selection */
  uint8_t oe         : 1;             /* True:  Output enable */
  uint8_t divsel     : 1;             /* True:  Clock source divider */
  uint8_t runstdby   : 1;             /* Run in standby */
  uint8_t source;                     /* GLCK clock source:
                                       *   0  XOSC 0 oscillator input
                                       *   1  XOSC 1 oscillator input
                                       *   2  Generator input pad
                                       *   3  Generic clock generator 1 output
                                       *   4  OSCULP32K oscillator output
                                       *   5  XOSC32K oscillator output
                                       *   6  DFLL oscillator output
                                       *   7  DPLL0 output
                                       *   8  DPLL1 output */
  uint16_t div;                       /* Division factor: 8-bits for all but GCLK1 */
};

/* This structure defines the configuration of the clock sus-system */

struct sam_clockconfig_s
{
  uint8_t waitstates;                 /* NVM read wait states 9-15 */
  uint8_t cpudiv;                     /* MCLK divider to get CPU frequency */
  uint16_t glckset1;                  /* GLCKs to initialize prior to DPLL init */
  uint16_t glckset2;                  /* GLCKs to initialize after to DPLL init */
  uint32_t cpu_frequency;             /* Resulting CPU frequency */
#if BOARD_HAVE_XOSC32K != 0
  struct sam_xosc32_config_s xosc32k; /* XOSC32 configuration */
#endif
#if BOARD_HAVE_XOSC0 != 0
  struct sam_xosc_config_s xosc0;     /* XOSC0 configuration */
#endif
#if BOARD_HAVE_XOSC1 != 0
  struct sam_xosc_config_s xosc1;     /* XOSC1 configuration */
#endif
  struct sam_dfll_config_s dfll;      /* DFLL configuration */
  struct sam_dpll_config_s dpll[2];   /* DPLL0/1 configurations */
  struct sam_gclk_config_s gclk[12];  /* GLCK configurations */
};

/************************************************************************************
 * Inline Functions
 ************************************************************************************/

#ifndef __ASSEMBLY__

/************************************************************************************
 * Public Data
 ************************************************************************************/

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/************************************************************************************
 * Public Function Prototypes
 ************************************************************************************/

/************************************************************************************
 * Name: sam_clock_configure
 *
 * Description:
 *   Configure the clock sub-system per the provided configuration data.
 *
 *   This should be called only (1) early in the initialization sequence, or (2)
 *   later but within a critical section.
 *
 ************************************************************************************/

struct sam_clockconfig_s;
void sam_clock_configure(const struct sam_clockconfig_s *config);

/************************************************************************************
 * Name: sam_clock_initialize
 *
 * Description:
 *   Configure the initial power-up clocking.  This function may be called
 *   only once by the power-up reset logic.
 *
 ************************************************************************************/

void sam_clock_initialize(void);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __ASSEMBLY__ */
#endif /* __ARCH_ARM_SRC_SAMD5E5_SAM_CLOCKCONFIG_H */
