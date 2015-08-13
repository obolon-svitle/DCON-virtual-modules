/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#include "lwip/def.h"
#include "fs.h"
#include <string.h>
#include "rest.h"
#include "restfsdata.h"

static struct rest_dev dev_not_found_htm;
static struct rest_dev dev_index_htm;

static struct rest_dev dev_index_htm = {
	.next = &dev_not_found_htm,
	.prev = NULL,
	.name = "index",
	.param_count = 0,
};

static struct rest_dev dev_not_found_htm = {
	.next = NULL,
	.prev = &dev_index_htm,
	.name = "404",
	.param_count = 0,
};

struct rest_dev *rest_root = &dev_index_htm;

/*-----------------------------------------------------------------------------------*/
/* Define the number of open files that we can support. */
#ifndef LWIP_MAX_OPEN_FILES
#define LWIP_MAX_OPEN_FILES     10
#endif

/* Define the file system memory allocation structure. */
struct fs_table {
	struct fs_file file;
	int inuse;
};

/* Allocate file system memory */
struct fs_table fs_memory[LWIP_MAX_OPEN_FILES];

/*-----------------------------------------------------------------------------------*/
static struct fs_file *fs_malloc(void) {
	int i;
	for(i = 0; i < LWIP_MAX_OPEN_FILES; i++) {
		if(fs_memory[i].inuse == 0) {
			fs_memory[i].inuse = 1;
			return(&fs_memory[i].file);
		}
	}
	return(NULL);
}

/*-----------------------------------------------------------------------------------*/
static void fs_free(struct fs_file *file) {
	int i;
	for(i = 0; i < LWIP_MAX_OPEN_FILES; i++) {
		if(&fs_memory[i].file == file) {
			fs_memory[i].inuse = 0;
			break;
		}
	}
	return;
}

/*-----------------------------------------------------------------------------------*/

struct fs_file *fs_open(const char *name) {
	struct fs_file *file;
	struct rest_dev *f;
	int namelen = 0;

	file = fs_malloc();
	if (file == NULL) {
		return NULL;
}
	#if !NO_SYS
	xSemaphoreTake(rest_root_mutex, portMAX_DELAY);
	#endif
	for (f = rest_root; f != NULL; f = f->next) {
		#if !NO_SYS
		xSemaphoreTake(f->semphr, portMAX_DELAY);
		#endif
		namelen = strlen(f->name);
		if (!strncmp(name, (char *)f->name, namelen)) {
			file->len = f->param_func(name + namelen, &file->data);
			file->index = file->len;
			file->pextension = NULL;
			file->semphr = f->semphr;
			#if !NO_SYS
			xSemaphoreGive(rest_root_mutex);
			#endif
			return file;
		}
		#if !NO_SYS
		xSemaphoreGive(f->semphr);
		#endif
	}
	#if !NO_SYS
	xSemaphoreGive(rest_root_mutex);
	#endif
  
	fs_free(file);
	return NULL;
}

/*-----------------------------------------------------------------------------------*/
void fs_close(struct fs_file *file) {
	#if !NO_SYS
	xSemaphoreGive(file->semphr);
	#endif
	
	fs_free(file);
}
/*-----------------------------------------------------------------------------------*/
int fs_read(struct fs_file *file, char *buffer, int count) {
	int read;

	if(file->index == file->len) {
		return -1;
	}

	read = file->len - file->index;
	if(read > count) {
		read = count;
	}

	memcpy(buffer, (file->data + file->index), read);
	file->index += read;

	return(read);
}

