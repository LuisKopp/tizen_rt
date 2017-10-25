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
 * examples/rda5981test/rda5981test.c
 *
 *   Copyright (C) 2008, 2011-2012 Gregory Nutt. All rights reserved.
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
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>
#include <stdio.h>
#include <unistd.h>
#include <apps/shell/tash.h>

/****************************************************************************
 * Definitions
 ****************************************************************************/
#define HELLO_TASH_PRI      100
#define HELLO_TASH_STAKSIZE 1024

/****************************************************************************
 * Private Data & Functions
 ****************************************************************************/
/* example */
#include <tinyara/i2c.h>
#include <tinyara/config.h>
#define RDA_WDT_BASE  ((0x40000000UL)+ 0x0000C)
#include "../arch/arm/src/rda5981/rda5981x_watchdog.h"
#include "../arch/arm/src/rda5981/rda5981x_pwm.h"

static struct i2c_dev_s *i2c_dev;
static struct i2c_config_s configs;

void i2c_test(void)
{
        uint8_t txbuf=0x03;
        uint8_t rxbuf[2];
        int ret;
	int port = 0;
        
	i2c_dev = up_i2cinitialize(port);
        if (i2c_dev == NULL) {
                printf("i2ctest_main: up_i2cinitialize(i2c:%d) failed\n", port);
		return;
        }

        configs.frequency = 100000;
        configs.address = 0x30;
        configs.addrlen = 7;

        ret = i2c_write(i2c_dev, &configs, &txbuf, 1);
        if (ret < 0) {
                printf("i2c_write fail(%d)\n", ret);
                return;
        }

	ret = i2c_read(i2c_dev, &configs, rxbuf, 2);
        if (ret < 0) {
                printf("i2c_read fail(%d)\n", ret);
                return;
        }
	printf("%02x %02x \r\n", rxbuf[0], rxbuf[1]);
	return;
}

void watchdog_test(void)
{

    wdt_t obj;
    int cnt = 0;

    rda_wdt_init(&obj, 2);
    rda_wdt_start(&obj);
    while(true) {
        printf("Alive loop\r\n");
        //mdelay(100); 
	if(cnt == 10) {
            while(true) {
                printf("Dead loop\r\n");
            }
        }
        cnt++;
        rda_wdt_feed(&obj);
    }

}

/*For I2S test*/
#if 0
#define BUFFER_SIZE 240
uint32_t rdata[BUFFER_SIZE] = {0};
i2s_t obj;
static void *i2s_receive(void *arg)
{
   while (true) {
        rda_i2s_int_recv(&obj, &rdata[0], BUFFER_SIZE);
        if (I2S_ST_BUSY == obj.sw_rx.state) {
            rda_i2s_sem_wait(i2s_rx_sem, osWaitForever);
        }
        printf("I2S a frame received,rdata[0]=%08X\n", rdata[0]);
    }
}

void i2s_master_in(void)
{
    printf("i2s_master_in test begin\n");
    pthread_t i2s_recv;
    pthread_attr_t attr;

    i2s_cfg_t cfg;
    cfg.mode              = I2S_MD_MASTER_RX;
    cfg.rx.fs             = I2S_64FS;
    cfg.rx.ws_polarity    = I2S_WS_NEG;
    cfg.rx.std_mode       = I2S_STD_M;
    cfg.rx.justified_mode = I2S_RIGHT_JM;
    cfg.rx.data_len       = I2S_DL_16b;
    cfg.rx.msb_lsb        = I2S_MSB;

    cfg.tx.fs             = I2S_64FS;
    cfg.tx.ws_polarity    = I2S_WS_NEG;
    cfg.tx.std_mode       = I2S_STD_M;
    cfg.tx.justified_mode = I2S_RIGHT_JM;
    cfg.tx.data_len       = I2S_DL_16b;
    cfg.tx.msb_lsb        = I2S_MSB;
    cfg.tx.wrfifo_cntleft = I2S_WF_CNTLFT_8W;

    rda_i2s_init(&obj, I2S_TX_BCLK, I2S_TX_WS, I2S_TX_SD, NC, NC, I2S_RX_SD, NC);
    rda_i2s_set_ws(&obj, 16000, 256);
    rda_i2s_set_tx_channel(&obj, 2);
    rda_i2s_set_rx_channel(&obj, 2);
    rda_i2s_format(&obj, &cfg);

    rda_i2s_enable_master_rx();
    rda_i2s_enable_rx(&obj);
    rda_i2s_enable_tx(&obj);
    rda_i2s_out_mute(&obj);

/*create recv thread*/
    pthread_create(&i2s_recv, &attr, i2s_receive, (void *)NULL);
    pthread_join(i2s_recv);
	
}
#endif

// For ADC test
float adc_read(void)
{
    float val=0;
    val = analogin_read();
    return val;	
}


unsigned short adc_read_u16(void)
{
    unsigned short val=0;
    val = analogin_read_u16();
    return val;
}


void adc_test(void)
{
    int read_times=0;
    int loop_count;
    printf("Start AnalogIn test...\r\n");
    analogin_init();
    while (read_times < 10) {
        float fval = adc_read();
        unsigned short ival = adc_read_u16();
        /* Print the percentage and 16-bit normalized values */
        printf("percentage: %3.3f%%\r\n", fval*100.0f);
        printf("normalized: 0x%04X\r\n\r\n", ival);
        loop_count = 100000000;
        while(loop_count>0)
	{
            loop_count--;
        }
        read_times++;
    }
}


/*For PWM test*/
struct pwmout_s pwm0;
struct pwmout_s pwm1;
struct pwmout_s pwm2;
struct pwmout_s pwm3;
struct pwmout_s pwm4;

void pwm_test(void) {

    pwmout_init(&pwm0, 0);
    pwmout_period_ms(&pwm0, 4.0f); 
    pwmout_write(&pwm0,0.25f);   


    pwmout_init(&pwm1, 1);
    pwmout_period_ms(&pwm1, 4.0f); 
    pwmout_write(&pwm1,0.50f);   
  
  
    pwmout_init(&pwm2, 2);
    pwmout_period_ms(&pwm2, 4.0f); 
    pwmout_write(&pwm2,0.75f);   
 
    pwmout_init(&pwm3, 3);
    pwmout_period_ms(&pwm3, 8.0f); 
    pwmout_write(&pwm3,0.25f);   
 
    pwmout_init(&pwm4, 4);
    pwmout_period_ms(&pwm4, 8.0f); 
    pwmout_write(&pwm4,0.50f);   
  
    while(true) {
        printf("duty=%f\r\n", pwmout_read(&pwm0));
    }
}

static void *hello_example(void *arg)
{
/*I2C TEST */
	//i2c_test();

/*WDG TEST*/
	//watchdog_test();

/*ADC TEST*/
	adc_test();

/*PWM TEST*/
	pwm_test();
	
/*I2S TEST*/
	//i2s_master_in();	
	//i2s_master_io();
	
	return NULL;
}

/*  Call-back function registered in TASH.
 *   This creates pthread to run an example with ASYNC TASH excution type.
 *   Only three points can be modified
 *   1. priority
 *   2. stacksize
 *   3. register entry function of pthread (example)
 */
static int hello_tash_cb(int argc, char **args)
{
	pthread_t hello_tash;
	pthread_attr_t attr;
	struct sched_param sparam;
	int status;
#ifdef SDCC
	pthread_addr_t result;
#endif

	/* Initialize the attribute variable */
	status = pthread_attr_init(&attr);
	if (status != 0) {
		printf("hello_tash : pthread_attr_init failed, status=%d\n", status);
	}

	/* 1. set a priority */
	sparam.sched_priority = HELLO_TASH_PRI;
	status = pthread_attr_setschedparam(&attr, &sparam);
	if (status != OK) {
		printf("hello_tash : pthread_attr_setschedparam failed, status=%d\n", status);
	}

	/* 2. set a stacksize */
	status = pthread_attr_setstacksize(&attr, HELLO_TASH_STAKSIZE);
	if (status != OK) {
		printf("hello_tash : pthread_attr_setstacksize failed, status=%d\n", status);
	}

	/* 3. create pthread with entry function */
	status = pthread_create(&hello_tash, &attr, hello_example, (void *)args);
	if (status != 0) {
		printf("hello_tash: pthread_create failed, status=%d\n", status);
	}

	/* Wait for the threads to stop */
#ifdef SDCC
	pthread_join(hello_tash, &result);
#else
	pthread_join(hello_tash, NULL);
#endif

	printf("hello_tash is finished\n");
	return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * hello_tash_main
 ****************************************************************************/
#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int rda5981_test_main(int argc, char **args)
#endif
{
	tash_cmd_install("rda5981_test", hello_tash_cb, TASH_EXECMD_ASYNC);

	return 0;
}
