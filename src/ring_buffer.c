// SPDX-License-Identifier: BSD-3-Clause

#include "ring_buffer.h"
#include <stdlib.h>
#include <string.h>
int ring_buffer_init(so_ring_buffer_t *ring, size_t cap)
{
	ring->data = malloc(cap);
	if(!ring->data)
		return -1;
	ring->cap = cap;
	ring->len = 0;
	ring->write_pos = 0;
	ring->read_pos = 0;
	/*Initialize synchronization primitives*/
	pthread_mutex_init(&ring->mutex, NULL);
	pthread_cond_init(&ring->empty, NULL);
	pthread_cond_init(&ring->full, NULL);
	ring->buffer_deschis = 1;
	return 0;
}

ssize_t ring_buffer_enqueue(so_ring_buffer_t *ring, void *data, size_t size)
{
	pthread_mutex_lock(&ring->mutex);
	/*Wait if there is not enough space for the new data*/
	while (ring->len + size > ring->cap && ring->buffer_deschis)
		pthread_cond_wait(&ring->full, &ring->mutex);
	if (!ring->buffer_deschis) {
		pthread_mutex_unlock(&ring->mutex);
		return -1;
	}
	/*Copy data into the ring buffer*/
	memcpy((char *)ring->data + ring->write_pos, data, size);
	ring->write_pos = (ring->write_pos + size);
	while (ring->write_pos >= ring->cap)
		ring->write_pos = ring->write_pos - ring->cap;
	ring->len = ring->len + size;
	/*Signal consumers that data is available*/
	pthread_cond_signal(&ring->empty);
	pthread_mutex_unlock(&ring->mutex);
	return size;
}

ssize_t ring_buffer_dequeue(so_ring_buffer_t *ring, void *data, size_t size)
{

	pthread_mutex_lock(&ring->mutex);
	/*Wait if there is not enough data to read*/
	while (ring->len < size && ring->buffer_deschis)
		pthread_cond_wait(&ring->empty, &ring->mutex);
	if (ring->len < size && !ring->buffer_deschis) {
		pthread_mutex_unlock(&ring->mutex);
		return -1;
	}
	/*Extract data from the ring buffer*/
	memcpy(data, (char *)ring->data + ring->read_pos, size);
	ring->read_pos = (ring->read_pos + size);
	while (ring->read_pos >= ring->cap)
		ring->read_pos = ring->read_pos - ring->cap;
	ring->len = ring->len - size;
	/*Signal producers that space is available*/
	pthread_cond_signal(&ring->full);
	pthread_mutex_unlock(&ring->mutex);
	return size;

}

void ring_buffer_destroy(so_ring_buffer_t *ring)
{

	free(ring->data);
	pthread_mutex_destroy(&ring->mutex);
	pthread_cond_destroy(&ring->empty);
	pthread_cond_destroy(&ring->full);
}

void ring_buffer_stop(so_ring_buffer_t *ring)
{

	pthread_mutex_lock(&ring->mutex);
	ring->buffer_deschis = 0;
	pthread_cond_broadcast(&ring->full);
	pthread_cond_broadcast(&ring->empty);
	pthread_mutex_unlock(&ring->mutex);
}
