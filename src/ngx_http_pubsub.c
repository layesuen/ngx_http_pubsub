/*
 * Copyright(C) Lei Sun, http://lsun.org
 */

#include "ngx_http_pubsub.h"

static char * ngx_http_pubsub(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void * ngx_http_pubsub_create_loc_conf(ngx_conf_t *cf);
static char * ngx_http_pubsub_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);
static ngx_int_t ngx_http_pubsub_handler(ngx_http_request_t *r);
static void ngx_http_pubsub_body_handler(ngx_http_request_t *r);

/* DIRECTIVES */

static ngx_command_t ngx_http_pubsub_commands[] = {

    {
        ngx_string("pubsub"),
        NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,
        ngx_http_pubsub,
        0,
        0,
        NULL
    }

};

/* HTTP MODULE CONTEXT */

static ngx_http_module_t module_ctx = {

    NULL,                                   /* preconfiguration */
    NULL,                                   /* postconfiguration */

    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */

    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */

    ngx_http_pubsub_create_loc_conf,        /* create location configuration */
    ngx_http_pubsub_merge_loc_conf          /* merge location configuration */

};


/* MODULE DEFINITION */

ngx_module_t ngx_http_pubsub_module = {

    NGX_MODULE_V1,                  
    &module_ctx,                        /* module context */
    ngx_http_pubsub_commands,           /* module directives */
    NGX_HTTP_MODULE,                    /* module type */
    NULL,                               /* init master */
    NULL,                               /* init module */
    NULL,                               /* init process */
    NULL,                               /* init thread */
    NULL,                               /* exit thread */
    NULL,                               /* exit process */
    NULL,                               /* exit master */
    NGX_MODULE_V1_PADDING

};

static char *
ngx_http_pubsub(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {

    ngx_http_core_loc_conf_t    *clcf;
    
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_pubsub_handler;

    return NGX_CONF_OK;

}

static void *
ngx_http_pubsub_create_loc_conf(ngx_conf_t *cf) {

    ngx_http_pubsub_loc_conf_t *lcf;

    lcf = ngx_palloc(cf->pool, sizeof(ngx_http_pubsub_loc_conf_t));
    if (lcf == NULL) {
        return NGX_CONF_ERROR;
    }
    ngx_memzero(lcf, sizeof(ngx_http_pubsub_loc_conf_t));
    
    return lcf;

}

static char *
ngx_http_pubsub_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child) {

    return NGX_CONF_OK;

}

static ngx_int_t
ngx_http_pubsub_handler(ngx_http_request_t *r) {

    ngx_http_pubsub_loc_conf_t      *lcf;
    ngx_int_t                       rc;

    lcf = ngx_http_get_module_loc_conf(r, ngx_http_pubsub_module);
    
    /* send header */
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_type.len = 10;
    r->headers_out.content_type.data = (void*)"text/plain";
    ngx_http_send_header(r);

    if (r->method == NGX_HTTP_GET) {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
            "ngx_http_pubsub subscriber");
        /* save subscriber's request */
        if (lcf->latest_subscriber) {
            lcf->latest_subscriber->keepalive = 0;
            ngx_http_finalize_request(lcf->latest_subscriber, NGX_DONE);
        }
        lcf->latest_subscriber = r;
        /* increase reference count */
        r->main->count++;
        return NGX_OK;
    }
    
    if (r->method == NGX_HTTP_POST) {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
            "ngx_http_pubsub publisher");
        r->request_body_in_single_buf = 1;
        r->request_body_in_persistent_file = 1;
        r->request_body_in_clean_file = 0;
        r->request_body_file_log_level = 0;

        rc = ngx_http_read_client_request_body(r, ngx_http_pubsub_body_handler);
        r->keepalive = 0;
        if (rc >= NGX_HTTP_SPECIAL_RESPONSE)
            return rc;
        else
            return NGX_OK;

    }

    return NGX_DONE;

}

static void
ngx_http_pubsub_body_handler(ngx_http_request_t *r) {

    ngx_http_pubsub_loc_conf_t      *lcf;
    ngx_buf_t                       *b;
    ngx_chain_t                     out;

    lcf = ngx_http_get_module_loc_conf(r, ngx_http_pubsub_module);

    if (r->request_body->bufs->buf != NULL) {
        b = ngx_create_temp_buf(lcf->latest_subscriber->pool, 0);
        *b = *(r->request_body->bufs->buf);
        b->last_buf = 0;
        b->flush = 1;
        out.buf = b;
        out.next = NULL;
        ngx_http_finalize_request(r, ngx_http_output_filter(lcf->latest_subscriber, &out));
    }

}

