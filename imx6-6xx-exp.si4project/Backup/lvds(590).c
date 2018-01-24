/*
 * $QNXLicenseC:
 * Copyright 2013, QNX Software Systems.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not reproduce, modify or distribute this software except in
 * compliance with the License. You may obtain a copy of the License
 * at: http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * This file may contain contributions from others, either as
 * contributors under the License or as licensors under other terms.
 * Please review this entire file for other proprietary rights or license
 * notices, as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#include <wfdqnx/wfdcfg.h>
#include <wfdqnx/wfdcfg_imx6x.h>
#include <stddef.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>  // NULL
#include <string.h>

struct wfdcfg_device {
	const struct wfdcfg_keyval *ext_list;
};

struct wfdcfg_port {
	int id;
	const struct wfdcfg_keyval *ext_list;
};

struct wfdcfg_mode_list {
	const struct mode *first_mode;
};

static const struct wfdcfg_keyval vdieo_SWPG_ext [] ={
	{.key = TIMING_LDB_OPTIONS, .i = TIMING_LDB_DATA_MAPPING_SPWG},
	{.key = TIMING_OUTPUT_FORMAT, .i = TIMING_OUTPUT_FORMAT_RGB888},
	{.key = NULL}
};
// Internal structure to keep a mode and its associated extension(s).
struct mode {
	const struct wfdcfg_timing timing;
	const struct wfdcfg_keyval *ext;
};


/* Helper functions */

static const struct mode*
cast_timing_to_mode(const struct wfdcfg_timing *timing)
{
	char *p = (char*)timing;
	if (p) {
		p -= offsetof(struct mode, timing);
	}
	return (const struct mode*)p;
}

static const struct wfdcfg_keyval*
get_ext_from_list(const struct wfdcfg_keyval *ext_list, const char *key)
{
	while (ext_list) {
		if (!ext_list->key) {
			ext_list = NULL;
			break;
		} else if (strcmp(ext_list->key, key) == 0) {
			return ext_list;
		}
		++ext_list;
	}
	return NULL;
}


static const struct wfdcfg_keyval device_ext[] = {
	{ NULL }  // marks end of list
};

static const struct wfdcfg_keyval port_ext[] = {
	{ NULL }  // marks end of list
};

#define MODELINE(mhz, h1, h2, h3, h4, v1, v2, v3, v4, f) \
	{ .pixel_clock_kHz = (mhz) * 1000, \
	.hpixels = (h1), .hbp = (h2) - (h1), .hsw = (h3) - (h2), .hfp = (h4) - (h3), \
	.vlines  = (v1), .vbp = (v2) - (v1), .vsw = (v3) - (v2), .vfp = (v4) - (v3), \
	.flags = (f), } \

static const struct mode modes[] = {
	/*
		T15 settings --- joshua@citos.com
	*/
	{
		.timing = MODELINE(51.2, 1024, 1124, 1244, 1344, 600, 610, 625, 635, 0),
		.ext = vdieo_SWPG_ext
	},
	/* A combination of the 800x600 and 640x480 modelines */
	{
		.timing = MODELINE(40.0, 800, 840, 968, 1056, 480, 481, 484, 500, 0),
	},

	/* These are pulled from the EDID of a nearby monitor */

	/* 1920x1200 (0xb4)  154.0MHz +HSync -VSync +preferred
		h: width  1920 start 1968 end 2000 total 2080 skew    0 clock   74.0KHz
		v: height 1200 start 1203 end 1209 total 1235           clock   60.0Hz */
	{
		.timing = MODELINE(154.0, 1920, 1968, 2000, 2080, 1200, 1203, 1209, 1235, WFDCFG_INVERT_VSYNC),
	},

	/* 1920x1080 (0xb6)  148.5MHz +HSync +VSync +preferred
		h: width  1920 start 2008 end 2052 total 2200 skew    0 clock   67.5KHz
		v: height 1080 start 1084 end 1089 total 1125           clock   60.0Hz */
	{
		.timing = MODELINE(148.5, 1920, 2008, 2052, 2200, 1080, 1084, 1089, 1125, 0),
	},

	/* 1920x720 @ 60Hz 97.34MHz +HSync +VSync
		h: width  1920 start 1980 end 2000 total 2080 skew    0
		v: height  720 start  722 end  723 total  780           */
	{
		.timing = MODELINE(97.34, 1920, 1980, 2000, 2080, 720, 722, 723, 780, 0),
	},
	{
		// marks end of list
		.timing = {.pixel_clock_kHz = 0},
	},
};


int
wfdcfg_device_create(struct wfdcfg_device **device, int deviceid,
		const struct wfdcfg_keyval *opts)
{
	int err = EOK;
	struct wfdcfg_device *tmp_dev = NULL;
	(void)opts;

	switch(deviceid) {
		case 1:
			tmp_dev = malloc(sizeof(*tmp_dev));
			if (!tmp_dev) {
				err = ENOMEM;
				goto end;
			}

			tmp_dev->ext_list = device_ext;
			break;

		default:
			/* Invalid device id*/
			err = ENOENT;
			goto end;
	}

end:
	if (err) {
		free(tmp_dev);
	} else {
		*device = tmp_dev;
	}
	return err;
}

const struct wfdcfg_keyval*
wfdcfg_device_get_extension(const struct wfdcfg_device *device,
		const char *key)
{
	return get_ext_from_list(device->ext_list, key);
}

void
wfdcfg_device_destroy(struct wfdcfg_device *device)
{
	free(device);
}

int
wfdcfg_port_create(struct wfdcfg_port **port,
		const struct wfdcfg_device *device, int portid,
		const struct wfdcfg_keyval *opts)
{
	int err = EOK;
	struct wfdcfg_port *tmp_port = NULL;
	(void)opts;

	assert(device);

	switch(portid) {
		// Ignore the port ID as all ports are equal
		default:
			tmp_port = malloc(sizeof(*tmp_port));
			if (!tmp_port) {
				err = ENOMEM;
				goto end;
			}
			tmp_port->id = portid;
			tmp_port->ext_list = port_ext;
			break;
		case 0:  // invalid
			err = ENOENT;
			goto end;
	}

end:
	if (err) {
		free(tmp_port);
	} else {
		*port = tmp_port;
	}
	return err;
}

const struct wfdcfg_keyval*
wfdcfg_port_get_extension(const struct wfdcfg_port *port, const char *key)
{
	return get_ext_from_list(port->ext_list, key);
}

void
wfdcfg_port_destroy(struct wfdcfg_port *port)
{
	free(port);
}

int
wfdcfg_mode_list_create(struct wfdcfg_mode_list **list,
		const struct wfdcfg_port* port, const struct wfdcfg_keyval *opts)
{
	int err = 0;
	struct wfdcfg_mode_list *tmp_mode_list = NULL;

	(void)opts;
	assert(port); (void)port;

	tmp_mode_list = malloc(sizeof *tmp_mode_list);
	if (!tmp_mode_list) {
		err = ENOMEM;
		goto out;
	}
	tmp_mode_list->first_mode = &modes[0];
out:
	if (err) {
		free(tmp_mode_list);
	} else {
		*list = tmp_mode_list;
	}
	return err;
}

const struct wfdcfg_keyval*
wfdcfg_mode_list_get_extension(const struct wfdcfg_mode_list *list,
		const char *key)
{
	(void)list;
	(void)key;
	return NULL;
}

void
wfdcfg_mode_list_destroy(struct wfdcfg_mode_list *list)
{
	free(list);
}

const struct wfdcfg_timing*
wfdcfg_mode_list_get_next(const struct wfdcfg_mode_list *list,
		const struct wfdcfg_timing *prev_timing)
{
	assert(list);

	const struct mode *m = list->first_mode;
	if (prev_timing) {
		m = cast_timing_to_mode(prev_timing) + 1;
	}

	if (m->timing.pixel_clock_kHz == 0) {
		// end of list (this is not an error)
		m = NULL;
	}
	return &m->timing;
}

const struct wfdcfg_keyval*
wfdcfg_mode_get_extension(const struct wfdcfg_timing *timing,
		const char *key)
{
	const struct wfdcfg_keyval *ext = cast_timing_to_mode(timing)->ext;
	return get_ext_from_list(ext, key);
}


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn/product/branches/6.6.0/trunk/hardware/wfd/imx6x/wfdcfg/imx6x-hdmi/hdmi.c $ $Rev: 726755 $")
#endif
