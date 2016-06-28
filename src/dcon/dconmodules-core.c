#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "common.h"

#include "dcon/dcon_dev.h"
#include "dcon/dcon_data.h"

#include "devices/7050/7050.h"
#include "devices/7017/7017.h"
#include "devices/7024/7024.h"

#define IS_ALL_DEV_CMD(msg) (strlen(msg) == 3 && \
        msg[1] == '*' && msg[2] == '*')

#define DEFAULT_MODULE_STACK_SIZE 1024
#define MIN_CMD_SIZE 3

static xSemaphoreHandle dcon_root_mutex;
static struct dcon_dev* dcon_root = NULL;

int dcon_init(void) {
    if ((dcon_root_mutex = xSemaphoreCreateMutex()) == NULL)
        return -1;

    xTaskCreate(Task7050Function, "7050",
                DEFAULT_MODULE_STACK_SIZE, NULL, 1, NULL);

#ifndef PART_LM3S6965
    //Not supported yet on lm3s6965
    xTaskCreate(Task7017Function, "7017",
                DEFAULT_MODULE_STACK_SIZE, NULL, 1, NULL);
#endif

    xTaskCreate(Task7024Function, "7024",
                DEFAULT_MODULE_STACK_SIZE, NULL, 1, NULL);
    return 0;
}

int dcon_dev_register(struct dcon_dev *dev) {

    xSemaphoreTake(dcon_root_mutex, portMAX_DELAY);
    
    dev->mutex = xSemaphoreCreateMutex();
    if (!dev->mutex) {
    	goto mutex_err;
    }
            
    dev->dev_q = xQueueCreate(1, sizeof(struct msg));
    if (!dev->dev_q) {
    	goto devq_err;
    }

    dev->data_q = xQueueCreate(1, sizeof(struct msg));
    if (!dev->data_q) {
        goto dataq_err;
    }

    dev->prev = NULL;
    dev->next = dcon_root;
    if (dcon_root) {
        dcon_root->prev = dev;
    }
    dcon_root = dev;

    DVM_LOG_I("%d %d registered", dev->addr, dev->type);

    xSemaphoreGive(dcon_root_mutex);

    return 0;
  
dataq_err:
	vQueueDelete(dev->dev_q);
devq_err:
	vSemaphoreDelete(dev->mutex);
mutex_err:
    xSemaphoreGive(dcon_root_mutex);

    DVM_LOG_E("%d %d register err", dev->addr, dev->type);

    return -1;
}

void dcon_dev_unregister(struct dcon_dev *dev) {
    xSemaphoreTake(dcon_root_mutex, portMAX_DELAY);
    
    vQueueDelete(dev->dev_q);
    vQueueDelete(dev->data_q);
    vSemaphoreDelete(dev->mutex);

    if (dev->prev) {
    	dev->prev->next = dev->next;
    }
    if (dev->next) {
    	dev->next->prev = dev->prev;
    }

    xSemaphoreGive(dcon_root_mutex);

    DVM_LOG_I("%d %d unregistered", dev->addr, dev->type);
}

void dcon_dev_recv(struct dcon_dev *dev, struct msg *msg) {
    xQueueReceive(dev->data_q, msg, portMAX_DELAY);
}

void dcon_dev_send(struct dcon_dev *dev, const struct msg *msgbuf) {
    xQueueSend(dev->dev_q, msgbuf, portMAX_DELAY);
}

void dcon_list_devices(void (*action)(int addr, int type,
                                      void *data), void *data) {
    xSemaphoreTake(dcon_root_mutex, portMAX_DELAY);
    for (struct dcon_dev *p = dcon_root; p != NULL; p = p->next) {
        action(p->addr, p->type, data);
    }
    xSemaphoreGive(dcon_root_mutex);
}

static struct dcon_dev* dcon_find_and_open(const char *request) {
    char addr_str[3];
    struct dcon_dev *result = NULL;
    
    xSemaphoreTake(dcon_root_mutex, portMAX_DELAY);

    for (struct dcon_dev *p = dcon_root; !result && p != NULL; p = p->next) {
        xSemaphoreTake(p->mutex, portMAX_DELAY);
        snprintf(addr_str, ARRAY_SIZE(addr_str), "%02x", p->addr);
        if (!strncmp(addr_str, request + 1, 2)) {
            result = p;
        }
        else
        {
            xSemaphoreGive(p->mutex);
        }
    }

    xSemaphoreGive(dcon_root_mutex);

    return result;
}

static inline void dcon_close(struct dcon_dev *dev) {
    xSemaphoreGive(dev->mutex);
}

static inline void dcon_call_dev(struct dcon_dev *dev, struct msg *msg) {
    xQueueSend(dev->data_q, msg, portMAX_DELAY);
    xQueueReceive(dev->dev_q, msg, portMAX_DELAY);
}

static void dcon_call_all(struct msg *msg) {
    xSemaphoreTake(dcon_root_mutex, portMAX_DELAY);
    
    for (struct dcon_dev *p = dcon_root; p != NULL; p = p->next) {
        dcon_call_dev(p, msg);
    }
    
    xSemaphoreGive(dcon_root_mutex);
}

void dcon_data_send(const char *request, char *response) {
    struct msg msg;
    struct dcon_dev *dev;

    if (strlen(request) < MIN_CMD_SIZE)
    {
        response[0] = '\0';
        return;
    }

    msg.request = request;
    msg.response = response;

    if (IS_ALL_DEV_CMD(request)) {
        
        dcon_call_all(&msg);
        return;
    }
    
    if ((dev = dcon_find_and_open(request))) {
        dcon_call_dev(dev, &msg);
        dcon_close(dev);
    } else {
        response[0] = '\0';
    }
}
