/*
 * Copyright(C) Lei Sun, http://lsun.org
 */

#include "ngx_http_pubsub.h"
#include "ngx_http_pubsub_core.h"
#include "ngx_http_pubsub_queue.h"

/* private vars */

static ngx_str_t                    shm_zone_name = ngx_string("ngx_http_pubsub");
static ngx_shm_zone_t              *shm_zone = NULL;

/* functions declr */

static char        *ngx_http_pubsub(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void        *ngx_http_pubsub_create_loc_conf(ngx_conf_t *cf);
static char        *ngx_http_pubsub_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);
static ngx_int_t    ngx_http_pubsub_handler(ngx_http_request_t *r);
static ngx_int_t    ngx_http_pubsub_postconfiguration(ngx_conf_t *cf);
static ngx_int_t    ngx_http_pubsub_init_process(ngx_cycle_t *cycle);
static ngx_int_t    ngx_http_pubsub_shm_zone_init(ngx_shm_zone_t *shm_zone, void *data);

/* module directives */

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

/* http module context */

static ngx_http_module_t module_ctx = {

    NULL,                                   /* preconfiguration */
    ngx_http_pubsub_postconfiguration,      /* postconfiguration */

    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */

    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */

    ngx_http_pubsub_create_loc_conf,        /* create location configuration */
    ngx_http_pubsub_merge_loc_conf          /* merge location configuration */

};

/* module definition */

ngx_module_t ngx_http_pubsub_module = {

    NGX_MODULE_V1,                  
    &module_ctx,                        /* module context */
    ngx_http_pubsub_commands,           /* module directives */
    NGX_HTTP_MODULE,                    /* module type */
    NULL,                               /* init master */
    NULL,                               /* init module */
    ngx_http_pubsub_init_process,       /* init process */
    NULL,                               /* init thread */
    NULL,                               /* exit thread */
    NULL,                               /* exit process */
    NULL,                               /* exit master */
    NGX_MODULE_V1_PADDING

};

/* initialization after all http module configuration parsed */

static ngx_int_t
ngx_http_pubsub_postconfiguration(ngx_conf_t *cf) {

    size_t size = 1 << 16;

    if (shm_zone == NULL) {
        shm_zone = ngx_shared_memory_add(cf, &shm_zone_name, size, &ngx_http_pubsub_module);
        shm_zone->init = ngx_http_pubsub_shm_zone_init;
    }

    return NGX_OK;

}

/* module initialization after all configuration parsed
   cycle->log updated and shm_zone created*/

static ngx_int_t
ngx_http_pubsub_init_process(ngx_cycle_t *cycle) {

    return ngx_http_pubsub_core_init(cycle, shm_zone);

}

/* "pubsub" directive handler */

static char *
ngx_http_pubsub(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {

    ngx_http_core_loc_conf_t    *clcf;
    
    /* install handler */
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_pubsub_handler;

    return NGX_CONF_OK;

}

/* content phase handler */

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
        return NGX_DONE;
    }
    
    if (r->method == NGX_HTTP_POST) {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
            "ngx_http_pubsub publisher");
        /*rc = ngx_http_pubsub_publish(r);*/
        if (rc >= NGX_HTTP_SPECIAL_RESPONSE)
            return rc;
        else
            return NGX_DONE;

    }

    return NGX_DONE;

}

/* shared memory initializer */

static ngx_int_t
ngx_http_pubsub_shm_zone_init(ngx_shm_zone_t *shm_zone, void *data) {

    ngx_http_pubsub_queue_init(shm_zone);
    
    return NGX_OK;

}

/* to create loc_conf */

static void *
ngx_http_pubsub_create_loc_conf(ngx_conf_t *cf) {

    ngx_http_pubsub_loc_conf_t *lcf;

    lcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_pubsub_loc_conf_t));
    if (lcf == NULL) {
        return NGX_CONF_ERROR;
    }
    
    return lcf;

}

/* to merge loc_conf */

static char *
ngx_http_pubsub_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child) {

    return NGX_CONF_OK;

}


