/*
 * Copyright (C) Lei Sun, http://lsun.org
 */

#ifndef _NGX_HTTP_PUBSUB_CORE_H_INCLUDED_
#define _NGX_HTTP_PUBSUB_CORE_H_INCLUDED_

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#define NGX_HTTP_PUBSUB_POLL_INTERVAL 1000
    
/* functions */

ngx_int_t   ngx_http_pubsub_core_init(ngx_cycle_t *c, ngx_shm_zone_t *shm_zone);
ngx_int_t   ngx_http_pubsub_publish(u_char *data, size_t size);
ngx_int_t   ngx_http_pubsub_subscribe(ngx_http_request_t *req);

#endif /* _NGX_HTTP_PUBSUB_CORE_H_INCLUDED_ */

