/****************************************************************************
 * arch/arm/src/armv7-m/up_systick.c
 *
 *   Copyright (C) 2018 Pinecone Inc. All rights reserved.
 *   Author: Xiang Xiao <xiaoxiang@pinecone.net>
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
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <nuttx/arch.h>
#include <nuttx/irq.h>

#include <stdio.h>

#include "nvic.h"
#include "systick.h"
#include "up_arch.h"

#ifdef CONFIG_ARMV7M_SYSTICK

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define NVIC_IRQ_SYSTICK    15          /* Vector 15: System tick */

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* This structure provides the private representation of the "lower-half"
 * driver state structure.  This structure must be cast-compatible with the
 * timer_lowerhalf_s structure.
 */

struct systick_lowerhalf_s
{
  const struct timer_ops_s *ops;        /* Lower half operations */
  uint32_t                  freq;       /* Timer working clock frequency(Hz) */
  tccb_t                    callback;   /* Current user interrupt callback */
  void                     *arg;        /* Argument passed to upper half callback */
  uint32_t                 *next_interval;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int systick_start(FAR struct timer_lowerhalf_s *lower_);
static int systick_stop(FAR struct timer_lowerhalf_s *lower_);
static int systick_getstatus(FAR struct timer_lowerhalf_s *lower_,
                             FAR struct timer_status_s *status);
static int systick_settimeout(FAR struct timer_lowerhalf_s *lower_,
                              uint32_t timeout);
static void systick_setcallback(FAR struct timer_lowerhalf_s *lower_,
                                tccb_t callback, FAR void *arg);
static int systick_maxtimeout(FAR struct timer_lowerhalf_s *lower_,
                              uint32_t *maxtimeout);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct timer_ops_s g_systick_ops =
{
  .start       = systick_start,
  .stop        = systick_stop,
  .getstatus   = systick_getstatus,
  .settimeout  = systick_settimeout,
  .setcallback = systick_setcallback,
  .maxtimeout  = systick_maxtimeout,
};

static struct systick_lowerhalf_s g_systick_lower =
{
  .ops = &g_systick_ops,
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static inline uint64_t usec_from_count(uint32_t count, uint32_t freq)
{
  return (uint64_t)count * USEC_PER_SEC / freq;
}

static inline uint64_t usec_to_count(uint32_t usec, uint32_t freq)
{
  return (uint64_t)usec * freq / USEC_PER_SEC;
}

static bool systick_is_running(void)
{
  return !!(getreg32(NVIC_SYSTICK_CTRL) & NVIC_SYSTICK_CTRL_ENABLE);
}

static bool systick_irq_pending(struct systick_lowerhalf_s *lower)
{
  if (lower->next_interval)
    {
      return false; /* Interrupt is in process */
    }
  else
    {
      return !!(getreg32(NVIC_INTCTRL) & NVIC_INTCTRL_PENDSTSET);
    }
}

static int systick_start(FAR struct timer_lowerhalf_s *lower_)
{
  putreg32(0, NVIC_SYSTICK_CURRENT);
  modifyreg32(NVIC_SYSTICK_CTRL, 0, NVIC_SYSTICK_CTRL_ENABLE);
  return 0;
}

static int systick_stop(FAR struct timer_lowerhalf_s *lower_)
{
  modifyreg32(NVIC_SYSTICK_CTRL, NVIC_SYSTICK_CTRL_ENABLE, 0);
  return 0;
}

static int systick_getstatus(FAR struct timer_lowerhalf_s *lower_,
                             FAR struct timer_status_s *status)
{
  struct systick_lowerhalf_s *lower = (struct systick_lowerhalf_s *)lower_;
  irqstate_t flags = enter_critical_section();

  status->flags    = lower->callback ? TCFLAGS_HANDLER : 0;
  status->flags   |= systick_is_running() ? TCFLAGS_ACTIVE : 0;
  status->timeout  = usec_from_count(getreg32(NVIC_SYSTICK_RELOAD),
                                     lower->freq);
  status->timeleft = usec_from_count(getreg32(NVIC_SYSTICK_CURRENT),
                                     lower->freq);

  if (systick_irq_pending(lower))
    {
      /* Interrupt is pending and the timer wrap happen? */

      if (status->timeleft)
        {
          /* Make timeout-timeleft equal the real elapsed time */

          status->timeout += status->timeout - status->timeleft;
          status->timeleft = 0;
        }
    }
  else if (status->timeleft == 0)
    {
      status->timeleft = status->timeout;
    }

  leave_critical_section(flags);
  return 0;
}

static int systick_settimeout(FAR struct timer_lowerhalf_s *lower_,
                              uint32_t timeout)
{
  struct systick_lowerhalf_s *lower = (struct systick_lowerhalf_s *)lower_;

  irqstate_t flags = enter_critical_section();
  if (lower->next_interval)
    {
      /* If the timer callback is in the process,
       * delay the update to timer interrupt handler.
       */

      *lower->next_interval = timeout;
    }
  else
    {
      uint32_t reload;

      reload = usec_to_count(timeout, lower->freq);
      putreg32(reload, NVIC_SYSTICK_RELOAD);
      if (systick_is_running())
        {
          if (reload != getreg32(NVIC_SYSTICK_CURRENT))
            {
              putreg32(0, NVIC_SYSTICK_CURRENT);
            }
        }
    }

  leave_critical_section(flags);
  return 0;
}

static void systick_setcallback(FAR struct timer_lowerhalf_s *lower_,
                                CODE tccb_t callback, FAR void *arg)
{
  struct systick_lowerhalf_s *lower = (struct systick_lowerhalf_s *)lower_;

  irqstate_t flags = enter_critical_section();
  lower->callback  = callback;
  lower->arg       = arg;
  leave_critical_section(flags);
}

static int systick_maxtimeout(FAR struct timer_lowerhalf_s *lower_,
                              uint32_t *maxtimeout)
{
  uint64_t maxtimeout64 = usec_from_count(
    NVIC_SYSTICK_RELOAD_MASK, g_systick_lower.freq);

  if (maxtimeout64 > UINT32_MAX)
    {
      *maxtimeout = UINT32_MAX;
    }
  else
    {
      *maxtimeout = maxtimeout64;
    }

  return 0;
}

static int systick_interrupt(int irq, FAR void *context, FAR void *arg)
{
  struct systick_lowerhalf_s *lower = arg;

  if (lower->callback && systick_is_running())
    {
      uint32_t reload = getreg32(NVIC_SYSTICK_RELOAD);
      uint32_t interval = usec_from_count(reload, lower->freq);
      uint32_t next_interval = interval;

      lower->next_interval = &next_interval;
      if (lower->callback(&next_interval, lower->arg))
        {
          if (next_interval && next_interval != interval)
            {
              reload = usec_to_count(next_interval, lower->freq);
              putreg32(reload, NVIC_SYSTICK_RELOAD);
              putreg32(0, NVIC_SYSTICK_CURRENT);
            }
        }
      else
        {
          modifyreg32(NVIC_SYSTICK_CTRL, NVIC_SYSTICK_CTRL_ENABLE, 0);
        }

      lower->next_interval = NULL;
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

struct timer_lowerhalf_s *systick_initialize(bool coreclk,
                                             unsigned int freq, int minor)
{
  struct systick_lowerhalf_s *lower = (struct systick_lowerhalf_s *)&g_systick_lower;

  /* Calculate the working clock frequency if need */

  if (freq == 0)
    {
      uint32_t calib = getreg32(NVIC_SYSTICK_CALIB);
      uint32_t tenms = calib & NVIC_SYSTICK_CALIB_TENMS_MASK;
      lower->freq = 100 * (tenms + 1);
    }
  else
    {
      lower->freq = freq;
    }

  /* Disable SYSTICK, but enable interrupt */

  if (coreclk)
    {
      putreg32(NVIC_SYSTICK_CTRL_CLKSOURCE | NVIC_SYSTICK_CTRL_TICKINT, NVIC_SYSTICK_CTRL);
    }
  else
    {
      putreg32(NVIC_SYSTICK_CTRL_TICKINT, NVIC_SYSTICK_CTRL);
    }

  irq_attach(NVIC_IRQ_SYSTICK, systick_interrupt, lower);
  up_enable_irq(NVIC_IRQ_SYSTICK);

  /* Register the timer driver if need */

  if (minor >= 0)
    {
      char devname[32];

      sprintf(devname, "/dev/timer%d", minor);
      timer_register(devname, (struct timer_lowerhalf_s *)lower);
    }

  return (struct timer_lowerhalf_s *)lower;
}

#endif /* CONFIG_ARMV7M_SYSTICK */
