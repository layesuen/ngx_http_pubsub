/*
 * Copyright (C) Lei Sun, http://lsun.org
 */

#include "ngx_http_pubsub_queue.h"

ngx_int_t
ngx_http_pubsub_queue_init(ngx_shm_zone_t *queue_zone) {

    ngx_uint_t                  i;
    size_t                      msg_pool_size;
    ngx_http_pubsub_msg_t      *m;
    ngx_http_pubsub_queue_t    *queue;

    msg_pool_size = queue_zone->shm.size - sizeof(ngx_http_pubsub_queue_t);

    if (msg_pool_size < sizeof(ngx_http_pubsub_msg_t) * 3) {
        return NGX_ERROR;
    }

    queue = (ngx_http_pubsub_queue_t*)queue_zone->shm.addr;
    ngx_memzero(queue, sizeof(ngx_http_pubsub_queue_t));
    queue->capacity = msg_pool_size / sizeof(ngx_http_pubsub_msg_t);

    /* init free chain */
    m = (ngx_http_pubsub_msg_t*)(queue_zone->shm.addr + sizeof(ngx_http_pubsub_queue_t));
    for (i = 0; i < queue->capacity; i++) {
        m->next = queue->free;
        m->seq_no = 0;
        queue->free = m;
        m++;
    }

    return NGX_OK;

}

ngx_http_pubsub_msg_t *
ngx_http_pubsub_queue_put(ngx_http_pubsub_queue_t *queue, size_t size) {

    ngx_http_pubsub_msg_t  *msg;

    ngx_spinlock(&(queue->free_lock), 1, 0xffff);
    msg = queue->free;
    if (queue->free != NULL) {
        queue->free = queue->free->next;
    }
    ngx_unlock(&(queue->free_lock));
    
    if (msg == NULL) return NULL;

    /* the msg in used chain are always in an order of increasing seq_no */
    ngx_spinlock(&(queue->used_lock), 1, 0xffff);
    msg->seq_no = queue->max_seq_no++;
    msg->next = queue->used;
    queue->used = msg;
    queue->size++;
    ngx_unlock(&(queue->used_lock));

    return msg;

}

void
ngx_http_pubsub_queue_free(ngx_http_pubsub_queue_t *queue, ngx_uint_t new_size) {

    ngx_http_pubsub_msg_t  *h, *t;

    ngx_spinlock(&(queue->used_lock), 1, 0xffff);
    h = queue->used;
    t = queue->used;
    while (new_size < queue->size) {
        t = t->next;
        queue->size--;
    }
    queue->used = t;
    ngx_unlock(&(queue->used_lock));

    ngx_spinlock(&(queue->free_lock), 1, 0xffff);
    t->next = queue->free;
    queue->free = h;
    ngx_unlock(&(queue->used_lock));

}

