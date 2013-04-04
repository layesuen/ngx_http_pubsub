/*
 * Copyright (C) Lei Sun, http://lsun.org
 *
 * The publisher that work in a polling manner
 *
 */


#include "ngx_http_pubsub_core.h"
#include "ngx_http_pubsub_queue.h"

/* private vars */

static ngx_http_pubsub_queue_t     *queue;                  /* shared among workers */
static ngx_uint_t                   queue_rd_seq_no = 0;    /* owned by individual worker */

static ngx_event_t                  publish_event;
static ngx_connection_t             dummy_conn;

static ngx_cycle_t                 *cycle;
//static ngx_http_pubsub_sub_chain   *subs;

/* function declr */

static void     ngx_http_pubsub_publish_handler(ngx_event_t *ev);

ngx_int_t
ngx_http_pubsub_core_init(ngx_cycle_t *c, ngx_shm_zone_t *shm_zone) {

    queue_rd_seq_no = 0;
    cycle = c;
    queue = (ngx_http_pubsub_queue_t*)shm_zone->shm.addr;
    
    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, cycle->log, 0,
        "ngx_http_pubsub_core_init");

    /* initialize publish event */
    dummy_conn.fd = -1024;
    publish_event.handler = ngx_http_pubsub_publish_handler;
    publish_event.data = &dummy_conn;
    publish_event.log = cycle->log;
    ngx_add_timer(&publish_event, NGX_HTTP_PUBSUB_POLL_INTERVAL);
    
    return NGX_OK;

}

ngx_int_t
ngx_http_pubsub_publish(u_char *data, size_t size) {

    /* if queue is 2/3 full, clear slow subscribers */
    /* add data to queue */
    
    return NGX_OK;

}

ngx_int_t
ngx_http_pubsub_subscribe(ngx_http_request_t *req) {

    return NGX_OK;

}

static void
ngx_http_pubsub_publish_handler(ngx_event_t *ev) {
    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, ev->log, 0,
        "ngx_http_pubsub_publish_handler");
    while (queue->max_seq_no != queue_rd_seq_no) {
       /* output for all subscriber */
       queue_rd_seq_no++;
    }
    ngx_add_timer(&publish_event, NGX_HTTP_PUBSUB_POLL_INTERVAL);
}


/*
ngx_int_t
ngx_http_pubsub_publish(ngx_http_request_t *r) {

    ngx_int_t rc;
    
    r->request_body_in_single_buf = 1;
    r->request_body_in_persistent_file = 1;
    r->request_body_in_clean_file = 0;
    r->request_body_file_log_level = 0;
    rc = ngx_http_read_client_request_body(r, ngx_http_pubsub_body_handler);
    r->keepalive = 0;

    return rc;

}

static void
ngx_http_pubsub_body_handler(ngx_http_request_t *r) {

    //ngx_http_pubsub_loc_conf_t      *lcf;
    //ngx_buf_t                       *b;
    //ngx_chain_t                     out;

    //lcf = ngx_http_get_module_loc_conf(r, ngx_http_pubsub_module);

    if (r->request_body->bufs->buf != NULL) {
        publish_event.log = r->connection->log;
        ngx_add_timer(&publish_event, 10);
        ngx_http_finalize_request(r, NGX_OK);
    }

}
*/

