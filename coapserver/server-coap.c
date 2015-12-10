#include "coap/coap.h"
#include "coap/resource.h"

#include "dcon/dcon_data.h"

#define INDEX "Virtual I-7000, I-8000 modules\n"

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

/*****************************************************************************/

static void hnd_get_index(coap_context_t *ctx ,
						 struct coap_resource_t *resource,
						 const coap_endpoint_t *local_interface,
						 coap_address_t *peer,
						 coap_pdu_t *request,
						 str *token,
						 coap_pdu_t *response) {
	unsigned buf[3];

	  response->hdr->code = COAP_RESPONSE_CODE(205);
	
	  coap_add_option(response,
					  COAP_OPTION_CONTENT_TYPE,
					  coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
	  
	  coap_add_option(response,
					  COAP_OPTION_MAXAGE,
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
		/* FIXME: need to check for resource->dirty? */
		coap_add_option(response,
						COAP_OPTION_OBSERVE,
						coap_encode_var_bytes(buf, ctx->observe), buf);
	}
	
	coap_add_option(response,
					COAP_OPTION_CONTENT_FORMAT,
					coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);

	coap_add_option(response,
					COAP_OPTION_MAXAGE,
					coap_encode_var_bytes(buf, 0x01), buf);

	if (request != NULL) {
		
		coap_get_data(request, &request_len, &request_buf);

		/* FIXME: add check for overflow*/
		request_buf[request_len] = '\0';
		
		dcon_data_send(request_buf, buf);

		len = snprintf((char *)buf,
					   min(sizeof(buf),
						   response->max_size - response->length),
					   buf);
		coap_add_data(response, len, buf);
	}

}

static void init_resources(coap_context_t* ctx) {
	coap_resource_t *r;

 	r = coap_resource_init(NULL, 0, 0);
	coap_register_handler(r, COAP_REQUEST_GET, hnd_get_index);

	coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
	coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"General Info\"", 14, 0);
	coap_add_resource(ctx, r);

	r = coap_resource_init((unsigned char *)"dcon", 4, COAP_RESOURCE_FLAGS_NOTIFY_CON);
	coap_register_handler(r, COAP_REQUEST_POST, hnd_get_module_data);

	coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
	coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"DCON Virtual Module\"", 16, 0);
	coap_add_resource(ctx, r);
}

static coap_context_t*  server_coap_init(void) {
	coap_context_t *newcontext;	
	coap_address_t listenaddress;

	coap_address_init(&listenaddress);

	/* looks like a server address, but is used as end point for clients too */
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
