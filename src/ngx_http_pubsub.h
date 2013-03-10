/*
 * Copyright (C) Lei Sun, http://lsun.org
 */

#ifndef _NGX_HTTP_PUBSUB_H_INCLUDED_
#define _NGX_HTTP_PUBSUB_H_INCLUDED_

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {

    ngx_http_request_t      *latest_subscriber;

} ngx_http_pubsub_loc_conf_t;


#endif /* _NGX_HTTP_PUBSUB_H_INCLUDED_ */

