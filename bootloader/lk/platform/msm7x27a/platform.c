/*
 * Copyright (c) 2008, Google Inc.
 * All rights reserved.
 *
 * Copyright (c) 2009-2011, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in
 *	the documentation and/or other materials provided with the
 *	distribution.
 *  * Neither the name of Google, Inc. nor the names of its contributors
 *	may be used to endorse or promote products derived from this
 *	software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <reg.h>
#include <debug.h>
#include <kernel/thread.h>
#include <platform/debug.h>
#include <platform/iomap.h>
#include <mddi.h>
#include <dev/fbcon.h>
#include <dev/gpio.h>
#include <smem.h>

static struct fbcon_config *fb_config;

static uint32_t ticks_per_sec = 0;

void platform_init_interrupts(void);
void platform_init_timer();

void uart3_clock_init(void);
void uart_init(void);

void acpu_clock_init(void);

void mddi_clock_init(unsigned num, unsigned rate);

extern void mipi_dsi_shutdown(void);

unsigned board_msm_id(void);

void platform_early_init(void)
{
#if WITH_DEBUG_UART
	uart1_clock_init();
	uart_init();
#endif
	platform_init_interrupts();
	platform_init_timer();
}

void platform_init(void)
{
	dprintf(INFO, "platform_init()\n");
	acpu_clock_init();
}

void display_init(void)
{
#if DISPLAY_TYPE_MDDI
	fb_config = mddi_init();
	ASSERT(fb_config);
	fbcon_setup(fb_config);
#endif
#if DISPLAY_TYPE_LCDC
	fb_config = lcdc_init();
	ASSERT(fb_config);
	fbcon_setup(fb_config);
#endif
#if DISPLAY_TYPE_MIPI
	dprintf(SPEW, "display_init()\n");
	panel_dsi_init();
	fb_config = mipi_init();
	ASSERT(fb_config);
	fbcon_setup(fb_config);
#endif
}

void display_shutdown(void)
{
#if DISPLAY_TYPE_MIPI
	if (machine_is_7x27a_evb())
		return;
	dprintf(SPEW, "display_shutdown()\n");
	mipi_dsi_shutdown();
	/* Power down DSI bridge chip */
	gpio_set(128, 0x1);
#endif
}

void platform_uninit(void)
{
#if DISPLAY_SPLASH_SCREEN
	display_shutdown();
#endif

	platform_uninit_timer();
}

/* Initialize DGT timer */
void platform_init_timer(void)
{
	/* disable timer */
	writel(0, DGT_ENABLE);

	ticks_per_sec = 19200000;	/* Uses TCXO (19.2 MHz) */
}

/* Returns timer ticks per sec */
uint32_t platform_tick_rate(void)
{
	return ticks_per_sec;
}

bool machine_is_7x25a(void)
{
	if ((board_msm_id() == MSM7225A) || (board_msm_id() == MSM7625A))
		return 1;
	else
		return 0;
}

/* Toggle RESET pin of the DSI Client before sending
 * panel init commands
 */
void panel_dsi_init(void)
{
	gpio_set(128, 0x1);
	mdelay(5);
	gpio_set(128, 0x0);
	gpio_set(129, 0x1);
	gpio_config(129, GPIO_OUTPUT);
	gpio_set(129, 0x0);
	gpio_set(129, 0x1);
	mdelay(10);
}
