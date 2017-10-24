/****************************************************************************
 *
 * Copyright 2016 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/
/****************************************************************************
 * arch/arm/src/rda5981/rda5981x_adc.c
 *
 *   Copyright (C) 2017 Gregory Nutt. All rights reserved.
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
 * 3. Neither the name tinyara nor the names of its contributors may be
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
#include <tinyara/config.h>

#include <sys/types.h>
#include <assert.h>
#include <debug.h>
#include <errno.h>

#include <tinyara/irq.h>
#include <tinyara/wqueue.h>
#include <tinyara/analog/adc.h>
#include <tinyara/analog/ioctl.h>

#include "up_arch.h"
#include "rda5981x_adc.h"
#include "chip/rda5981x_pinconfig.h"

//
#ifdef CONFIG_RDA5981_ADC

#define RDA_ADC_RANGE               (0x03FFU)

static int adc_inited_cnt = 0;


/****************************************************************************
 * Private Types
 ****************************************************************************/

/* This structure describes the state of one ADC block */
struct rda_dev_s {
	FAR const struct adc_callback_s *cb;

	struct adc_dev_s *dev;	/* A reference to the outer (parent) */
	uint8_t nchannels;	/* Number of channels */
	uint8_t cchannels;	/* Number of configured channels */
	uint8_t current;	/* Current ADC channel being converted */

	//struct work_s work;	/* Supports the IRQ handling */
	//uint8_t chanlist[RDA5981X_ADC_MAX_CHANNELS];
};

typedef enum {
    ADC0_0 = 0,
    ADC0_1,
    ADC0_2
} ADCName;


/****************************************************************************
 * Private Functions
 ****************************************************************************/
/****************************************************************************
 * Name: adc_conversion
 *
 * Description:
 *   This function executes on the worker thread.  It is scheduled by
 *   adc_interrupt whenever any enabled end-of-conversion event occurs.
 *   All interrupts are disabled when this function runs.
 *   adc_conversion will re-enable the interrupt when it completes
 *   processing the pending interrupt.
 *
 * Input Parameters
 *   arg - The ADC private data structure cast to (void *)
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/
static void adc_conversion(void *arg)
{
#if 0
	uint16_t sample;
	struct rda_dev_s *priv = (struct rda_dev_s *)arg;

	/* Read the ADC sample and pass it to the upper-half */
	sample = getreg32(S5J_ADC_DAT) & ADC_DAT_ADCDAT_MASK;

	if (priv->cb != NULL) {
		DEBUGASSERT(priv->cb->au_receive != NULL);
		priv->cb->au_receive(priv->dev,
				priv->chanlist[priv->current], sample);
	}

	/* Set the next channel to be sampled */
	priv->current++;

	if (priv->current >= priv->nchannels)
		priv->current = 0;

	/* Change to the next channel */
	modifyreg32(S5J_ADC_CON2, ADC_CON2_ACHSEL_MASK,
				priv->chanlist[priv->current]);

	/* Exit, start a new conversion */
	modifyreg32(S5J_ADC_CON1, 0, ADC_CON1_STCEN_ENABLE);
#endif
}

/****************************************************************************
 * Name: adc_interrupt
 *
 * Description:
 *   Common ADC interrupt handler.
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/
static int adc_interrupt(int irq, FAR void *context, void *arg)
{
#if 0

	int ret;
	FAR struct rda_dev_s *priv = (FAR struct rda_dev_s *)arg;

	if (getreg32(S5J_ADC_INT_STATUS) & ADC_INT_STATUS_PENDING) {
		/* Clear interrupt pending */
		putreg32(ADC_INT_STATUS_PENDING, S5J_ADC_INT_STATUS);

		/*
		 * Check if interrupt work is already queued. If it is already
		 * busy, then we already have interrupt processing in the
		 * pipeline and we need to do nothing more.
		 */
		if (work_available(&priv->work)) {
			ret = work_queue(LPWORK, &priv->work, adc_conversion,
							priv, 0);
			if (ret != 0) {
				lldbg("ERROR: failed to queue work: %d\n", ret);
			}
		}
	}
#endif
	return 0;
}

/****************************************************************************
 * Name: adc_startconv
 *
 * Description:
 *   Start (or stop) the ADC conversion process
 *
 * Input Parameters:
 *   priv   - A reference to the ADC block status
 *   enable - True: Start conversion
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/
static void adc_startconv(FAR struct rda_dev_s *priv, bool enable)
{
#if 0

	if (enable) {
		modifyreg32(S5J_ADC_CON1, 0, ADC_CON1_STCEN_ENABLE);
	} else {
		modifyreg32(S5J_ADC_CON1, ADC_CON1_STCEN_ENABLE, 0);
	}
#endif
}


/****************************************************************************
 * Name: adc_set_ch
 *
 * Description
 *   Sets the ADC channel
 *
 * Input Parameters:
 *   dev - pointer to device structure used by the drvier
 *   ch  - ADC channel number + 1 (0 is reserved for all configured channels.)
 *
 * Returned Value:
 *   int - errno
 *
 ****************************************************************************/
static int adc_set_ch(FAR struct adc_dev_s *dev, uint8_t ch)
{
#if 0

	int i;
	FAR struct rda_dev_s *priv = (FAR struct rda_dev_s *)dev->ad_priv;

	if (ch == 0) {
		/*
		 * 0 is reserved for special purpose which samples
		 * all channels
		 */
		priv->current = 0;
		priv->nchannels = priv->cchannels;
	} else {
		/* REVISIT: changing channel is not supported for now */

		for (i = 0; i < priv->cchannels &&
					priv->chanlist[i] != ch - 1; i++);

		if (i >= priv->cchannels) {
			return -ENODEV;
		}

		priv->current   = i;
		priv->nchannels = 1;
	}

	modifyreg32(S5J_ADC_CON2, ADC_CON2_ACHSEL_MASK,
				priv->chanlist[priv->current]);
#endif
	return 0;
}

/****************************************************************************
 * Name: adc_bind
 *
 * Description:
 *   Bind the upper-half driver callbacks to the lower-half implementation.
 *   This must be called early in order to receive ADC event notifications.
 *
 ****************************************************************************/
static int adc_bind(FAR struct adc_dev_s *dev,
		    FAR const struct adc_callback_s *callback)
{
#if 0

	FAR struct rda_dev_s *priv = (FAR struct rda_dev_s *)dev->ad_priv;

	DEBUGASSERT(priv != NULL);
	priv->cb = callback;
#endif
	return 0;
}

/****************************************************************************
 * Name: adc_reset
 *
 * Description:
 *   Reset the ADC device.  Called early to initialize the hardware. This
 *   is called, before adc_setup() and on error conditions.
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/
static void adc_reset(FAR struct adc_dev_s *dev)
{
#if 0

	irqstate_t flags;

	flags = irqsave();

	/* Reset ADC */
	putreg32(ADC_CON1_SOFTRESET_RESET, S5J_ADC_CON1);

	/* Release ADC from reset state */
	putreg32(ADC_CON1_SOFTRESET_NONRESET, S5J_ADC_CON1);

	/* Configuration of the channel conversions */
	adc_set_ch(dev, 0);

	irqrestore(flags);
#endif
}

/****************************************************************************
 * Name: adc_setup
 *
 * Description:
 *   Configure the ADC. This method is called the first time that the ADC
 *   device is opened.  This will occur when the port is first opened.
 *   This setup includes configuring and attaching ADC interrupts.
 *   Interrupts are all disabled upon return.
 *
 * Input Parameters:
 *
 * Returned Value:
 ****************************************************************************/
static int adc_setup(FAR struct adc_dev_s *dev)
{
#if 0

	int ret;
	FAR struct rda_dev_s *priv = (FAR struct rda_dev_s *)dev->ad_priv;

	/* Attach the ADC interrupt */
	ret = irq_attach(IRQ_ADC, adc_interrupt, priv);
	if (ret < 0) {
		lldbg("irq_attach failed: %d\n", ret);
		return ret;
	}

	/* Make sure that the ADC device is in the powered up, reset state */
	adc_reset(dev);

	/*
	 * Enable the ADC interrupt, but it will not be generated until we
	 * request to start the conversion.
	 */
	lldbg("Enable the ADC interrupt: irq=%d\n", IRQ_ADC);
	up_enable_irq(IRQ_ADC);
#endif
	return 0;
}

/****************************************************************************
 * Name: adc_shutdown
 *
 * Description:
 *   Disable the ADC.  This method is called when the ADC device is closed.
 *   This method reverses the operation the setup method.
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/
static void adc_shutdown(FAR struct adc_dev_s *dev)
{
#if 0

	/* Disable interrupt */
	putreg32(ADC_INT_DISABLE, S5J_ADC_INT);

	/* Disable ADC interrupts and detach the ADC interrupt handler */
	up_disable_irq(IRQ_ADC);
	irq_detach(IRQ_ADC);

	/* Reset ADC */
	putreg32(ADC_CON1_SOFTRESET_RESET, S5J_ADC_CON1);
#endif
}

/****************************************************************************
 * Name: adc_rxint
 *
 * Description:
 *   Call to enable or disable RX interrupts.
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/
static void adc_rxint(FAR struct adc_dev_s *dev, bool enable)
{
#if 0
	/* Enable or Disable ADC interrupt */
	putreg32(enable ? ADC_INT_ENABLE : ADC_INT_DISABLE, S5J_ADC_INT);
#endif
}

/****************************************************************************
 * Name: adc_ioctl
 *
 * Description:
 *   All ioctl calls will be routed through this method.
 *
 * Input Parameters:
 *   dev - pointer to device structure used by the driver
 *   cmd - command
 *   arg - arguments passed with command
 *
 * Returned Value:
 *
 ****************************************************************************/
static int adc_ioctl(FAR struct adc_dev_s *dev, int cmd, unsigned long arg)
{
#if 0

	FAR struct rda_dev_s *priv = (FAR struct rda_dev_s *)dev->ad_priv;
	int ret = OK;

	switch (cmd) {
	case ANIOC_TRIGGER:
		adc_startconv(priv, true);
		break;

	default:
		ret = -ENOTTY;
		break;
	}
#endif
	return 0;
}

/****************************************************************************
 * Private Data
 ****************************************************************************/
static const struct adc_ops_s g_adcops = {
	.ao_bind	= adc_bind,
	.ao_reset	= adc_reset,
	.ao_setup	= adc_setup,
	.ao_shutdown	= adc_shutdown,
	.ao_rxint	= adc_rxint,
	.ao_ioctl	= adc_ioctl,
};

static struct rda_dev_s g_adcpriv = {
	.cb		= NULL,
	.current	= 0,
};

static struct adc_dev_s g_adcdev;

/****************************************************************************
 * Public Functions
 ****************************************************************************/
void analogin_init(void) 
{
   //use Port B pin6
    unsigned char gp = 6U;  //PortB 6pin

    /* Init ADC */
    if(0 == adc_inited_cnt) {
        rda_ccfg_adc_init();
    }
    adc_inited_cnt++;

    rda_ccfg_gp(gp, 0x00U);
    rda_ccfg_adc_gp(gp, 0x00U);

    rda_configgpio(GPIO_RDA_ADC);
}

uint16_t analogin_read_u16(void) 
{printf("analogin_read_u16\n");
    uint16_t value = rda_ccfg_adc_read((unsigned char)ADC0_0);
    return (value); // 10 bit
}

float analogin_read(void) 
{
  printf("analogin_read\n");
    uint16_t value = rda_ccfg_adc_read((unsigned char)ADC0_0);
    return (float)value * (1.0f / (float)RDA_ADC_RANGE);
}

void analogin_free(void) 
{
    unsigned char gp = 6U;
    adc_inited_cnt--;
    if(0 == adc_inited_cnt) {
        rda_ccfg_adc_free();
    }
#if 0	// because we only use ADC0_0
    if(ADC0_2 == obj->adc) {
        return;
    }
    if(ADC0_1 == obj->adc) {
        if(0U == adc1_gp) {
            return;
        }
        gp = adc1_gp;
    }
#endif	
    rda_ccfg_adc_gp(gp, 0x01U);
    rda_ccfg_gp(gp, 0x01U);
}


/****************************************************************************
 * Name: rda5981x_adc_initialize
 *
 * Description:
 *   Initialize the ADC. As the pins of each ADC channel are exported through
 *   configurable GPIO and it is board-specific, information on available
 *   ADC channels should be passed to s5j_adc_initialize().
 *
 * Input Parameters:
 *
 * Returned Value:
 *   Valid ADC device structure reference on succcess; a NULL on failure
 *
 ****************************************************************************/
struct adc_dev_s *rda5981x_adc_initialize(void)
{
	FAR struct rda_dev_s *priv = &g_adcpriv;

	/* Initialize the public ADC device data structure */
	g_adcdev.ad_ops  = &g_adcops;
	g_adcdev.ad_priv = priv;

	/* Initialize the private ADC device data structure */
	priv->cb         = NULL;
	priv->dev        = &g_adcdev;
	//priv->cchannels  = cchannels;
#if 0
	if (cchannels > RDA5981X_ADC_MAX_CHANNELS) {
		lldbg("RDA5981 has maximum %d ADC channels.\n",
						RDA5981X_ADC_MAX_CHANNELS);
		return NULL;
	}

	memcpy(priv->chanlist, chanlist, cchannels);
#endif

	analogin_init(); 

	return &g_adcdev;
}
#endif /* CONFIG_RDA5981_ADC */
