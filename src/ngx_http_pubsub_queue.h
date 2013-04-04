/*
 * Lock-free queue
 * Copyright (C) Lei Sun, http://lsun.org
 */

#ifndef _NGX_HTTP_PUBSUB_QUEUE_H_INCLUDED_
#define _NGX_HTTP_PUBSUB_QUEUE_H_INCLUDED_

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_atomic.h>

#if (!NGX_HAVE_ATOMIC_OPS)
#error "doesn't have atomic ops"
#endif

#define NGX_HTTP_PUBSUB_MSG_FRAME_SIZE  128
#define NGX_HTTP_PUBSUB_MSG_DATA_SIZE   NGX_HTTP_PUBSUB_MSG_FRAME_SIZE \
     - sizeof(ngx_http_pubsub_msg_t*) \
     - sizeof(ngx_uint_t)

/* types */

typedef struct ngx_http_pubsub_msg_s ngx_http_pubsub_msg_t;

struct ngx_http_pubsub_msg_s {
   
    ngx_http_pubsub_msg_t      *next;
    ngx_uint_t                  seq_no;
    u_char                      data[NGX_HTTP_PUBSUB_MSG_DATA_SIZE];


};

typedef struct {

    size_t                      capacity;
    ngx_atomic_t                used_lock;
    ngx_http_pubsub_msg_t      *used;
    ngx_uint_t                  max_seq_no;
    size_t                      size;
    ngx_atomic_t                free_lock;
    ngx_http_pubsub_msg_t      *free;

} ngx_http_pubsub_queue_t;


/* functions */

ngx_int_t               ngx_http_pubsub_queue_init(ngx_shm_zone_t *queue_zone);
ngx_http_pubsub_msg_t * ngx_http_pubsub_queue_put(ngx_http_pubsub_queue_t *queue, size_t size);
void                    ngx_http_pubsub_queue_free(ngx_http_pubsub_queue_t *queue, ngx_uint_t new_size);

#endif /* _NGX_HTTP_PUBSUB_QUEUE_H_INCLUDED_ */


