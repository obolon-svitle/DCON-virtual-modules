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

static xSemaphoreHandle dcon_root_mutex;
static struct dcon_dev* dcon_root = NULL;

#define DEFAULT_MODULE_STACK_SIZE 200

int dcon_init(void) {
    if ((dcon_root_mutex = xSemaphoreCreateMutex()) == NULL)
        return -1;

    xTaskCreate(Task7050Function, "7050",
                DEFAULT_MODULE_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(Task7017Function, "7017",
                DEFAULT_MODULE_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(Task7024Function, "7024",
                DEFAULT_MODULE_STACK_SIZE, NULL, 1, NULL);
    return 0;
}

int dcon_dev_register(struct dcon_dev *dev) {
    xSemaphoreTake(dcon_root_mutex, portMAX_DELAY);
    
    dev->mutex = xSemaphoreCreateMutex();
    if (dev->mutex == NULL) {
        xSemaphoreGive(dcon_root_mutex);
        return -1;
    }
            
    dev->dev_q = xQueueCreate(1, sizeof(struct msg));
    dev->data_q = xQueueCreate(1, sizeof(struct msg));
    if (dev->dev_q == 0 || dev->data_q == 0) {
        xSemaphoreGive(dcon_root_mutex);
        return -1;
    }

    dev->prev = NULL;
    dev->next = dcon_root;
    if (dcon_root != NULL)
        dcon_root->prev = dev;
    dcon_root = dev;
  
    xSemaphoreGive(dcon_root_mutex);
    return 0;
}

void dcon_dev_unregister(struct dcon_dev *dev) {    
    xSemaphoreTake(dcon_root_mutex, portMAX_DELAY);
    xSemaphoreTake(dev->mutex, portMAX_DELAY);
    
    vQueueDelete(dev->dev_q);
    vQueueDelete(dev->data_q);

    dev->prev->next = dev->next;
    dev->next->prev = dev->prev;

    xSemaphoreGive(dev->mutex);
    xSemaphoreGive(dcon_root_mutex);
}

inline void dcon_dev_recv(struct dcon_dev *dev, struct msg *msg) {
    xQueueReceive(dev->data_q, msg, portMAX_DELAY);
}

inline void dcon_dev_send(struct dcon_dev *dev, const struct msg *msgbuf) {
    xQueueSend(dev->dev_q, msgbuf, portMAX_DELAY);
}

void dcon_list_devices(void (*action)(int addr, int type,
                                      void *data), void *data) {
    xSemaphoreTake(dcon_root_mutex, portMAX_DELAY);
    for (struct dcon_dev *p = dcon_root; p != NULL; p = p->next) {
        xSemaphoreTake(p->mutex, portMAX_DELAY);
        action(p->addr, p->type, data);
        xSemaphoreGive(p->mutex);
    }
    xSemaphoreGive(dcon_root_mutex);
}

static struct dcon_dev* dcon_find(const char *request) {
    char addr_str[3];
    if (strlen(request) < 3)
        return NULL;
    
    xSemaphoreTake(dcon_root_mutex, portMAX_DELAY);

    for (struct dcon_dev *p = dcon_root; p != NULL; p = p->next) {
        xSemaphoreTake(p->mutex, portMAX_DELAY);
        snprintf(addr_str, ARRAY_SIZE(addr_str), "%02x", p->addr);
        if (!strncmp(addr_str, request + 1, 2)) {
            xSemaphoreGive(dcon_root_mutex);
            return p;
        }
        xSemaphoreGive(p->mutex);
    }

    xSemaphoreGive(dcon_root_mutex);

    return NULL;
}

static void dcon_close(struct dcon_dev *dev) {
    xSemaphoreGive(dev->mutex);
}

static void dcon_call_dev(struct dcon_dev *dev, struct msg *msg) {
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

    msg.request = request;
    msg.response = response;

    if (strlen(request) == 3 && request[1] == '*' &&
        request[2] == '*') {
        
        dcon_call_all(&msg);
        return;
    }
    
    if ((dev = dcon_find(request)) != NULL) {
        dcon_call_dev(dev, &msg);
        dcon_close(dev);
    } else {
        response[0] = '\0';
    }
}
