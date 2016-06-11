#include "coap/coap.h"
#include "coap/resource.h"

#include "dcon/dcon_data.h"

#define INDEX "Virtual ICP-DAS I-7000 modules series\n"
#define DCON_NAME_RES "\"DCON virtual modules\""
#define DCON_MAIN_RES "dcon"
#define COLLECTION_ATTR "core.ll"

#define RES_MEM_SIZE 15

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

static void add_module(int addr, int type, coap_context_t *ctx);

static void hnd_get_index(coap_context_t *ctx ,
                          struct coap_resource_t *resource,
                          const coap_endpoint_t *local_interface,
                          coap_address_t *peer,
                          coap_pdu_t *request,
                          str *token,
                          coap_pdu_t *response) {
    unsigned buf[3];
    
    response->hdr->code = COAP_RESPONSE_CODE(205);
    
    coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
                    coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
      
    coap_add_option(response, COAP_OPTION_MAXAGE,
                    coap_encode_var_bytes(buf, 0x2ffff), buf);

    coap_add_data(response, strlen(INDEX), (unsigned char *)INDEX);
}

static void
hnd_get_module_data(coap_context_t  *ctx,
                    struct coap_resource_t *resource,
                    const coap_endpoint_t *local_interface,
                    coap_address_t *peer,
                    coap_pdu_t *request,
                    str *token,
                    coap_pdu_t *response) {
    size_t request_len;
    unsigned char *request_buf;
    unsigned char buf[DCON_MAX_BUF];
    size_t len;
    int result;

    response->hdr->code = COAP_RESPONSE_CODE(205);

    if (coap_find_observer(resource, peer, token)) {
        coap_add_option(response, COAP_OPTION_OBSERVE,
                        coap_encode_var_bytes(buf, ctx->observe), buf);
    }
    
    coap_add_option(response, COAP_OPTION_CONTENT_FORMAT,
                    coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);

    coap_add_option(response, COAP_OPTION_MAXAGE,
                    coap_encode_var_bytes(buf, 0x01), buf);

    if (request != NULL) {
        coap_get_data(request, &request_len, &request_buf);

        request_buf[request_len] = '\0';
        
        dcon_data_send(request_buf, buf);

        len = snprintf((char *)buf,
                       min(sizeof(buf),response->max_size - response->length), buf);
        coap_add_data(response, len, buf);
    }
}

static void init_resources(coap_context_t* ctx) {
    coap_resource_t *r;

    r = coap_resource_init(NULL, 0, 0);
    coap_register_handler(r, COAP_REQUEST_GET, hnd_get_index);

    r = coap_resource_init((unsigned char *)DCON_MAIN_RES, 4, COAP_RESOURCE_FLAGS_NOTIFY_CON);
    coap_register_handler(r, COAP_REQUEST_POST, hnd_get_module_data);

    coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
    coap_add_attr(r, (unsigned char *)"title", 5,
                  (unsigned char *)DCON_NAME_RES, sizeof(DCON_NAME_RES) - 1, 0);
    coap_add_resource(ctx, r);
    vTaskDelay(10000);

    dcon_list_devices(add_module, ctx);
}

static void add_module(int addr, int type, coap_context_t *ctx) {

    coap_resource_t *r;
    char *buf = mem_malloc(RES_MEM_SIZE);

    snprintf(buf, RES_MEM_SIZE, DCON_MAIN_RES"/%02x", addr);

    r = coap_resource_init((unsigned char *)buf, strlen(buf), COAP_RESOURCE_FLAGS_NOTIFY_CON);

    coap_register_handler(r, COAP_REQUEST_POST, hnd_get_module_data);
    buf = memp_malloc(RES_MEM_SIZE);
    snprintf(buf, RES_MEM_SIZE, "\"%02x\"", type);
    coap_add_attr(r, (unsigned char *)"rt", 2,
                  (unsigned char *) buf, strlen(buf), 0);
    coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
    coap_add_resource(ctx, r);
}

static coap_context_t*  server_coap_init(void) {
    coap_context_t *newcontext;    
    coap_address_t listenaddress;

    coap_address_init(&listenaddress);

    listenaddress.addr = *(IP_ADDR_ANY);
    listenaddress.port = COAP_DEFAULT_PORT;
    newcontext = coap_new_context(&listenaddress);

    LWIP_ASSERT("Failed to initialize context", newcontext != NULL);

    return newcontext;
}

void server_coap_serve(void) {
    coap_context_t *ctx;
    coap_tick_t now;
    coap_queue_t *nextpdu;

    ctx = server_coap_init();
    
    coap_set_log_level(LOG_WARNING);
    
    init_resources(ctx);
    
    for (;;) {
        nextpdu = coap_peek_next(ctx);

        coap_ticks(&now);
        while (nextpdu && nextpdu->t <= now - ctx->sendqueue_basetime) {
            coap_retransmit( ctx, coap_pop_next( ctx ) );
            nextpdu = coap_peek_next( ctx );            
        }
        coap_check_notify(ctx);
    }
}
