
#include <stdlib.h>
#include <string.h>

#include "circularbuffer.h"

#define Cb_available_data(B)	(((B)->bufferSize - (B)->tail+(B)->head) % (B)->bufferSize)
#define Cb_available_space(B)	((B)->bufferSize - 1 - Cb_available_data(B))
#define Cb_full(B)				(Cb_available_space((B))== 0) //< (B)->dataSize)
#define Cb_empty(B)				(Cb_available_data((B)) == 0)
#define Cb_starts_at(B)			((B)->base + ((B)->tail*(B)->dataSize))
#define Cb_ends_at(B)			((B)->base + ((B)->head*(B)->dataSize))
#define Cb_commit_read(B)		((B)->tail = ((B)->tail + 1) % (B)->bufferSize)
#define Cb_commit_write(B)		((B)->head = ((B)->head + 1) % (B)->bufferSize)

pCircularBuffer_t Cb_initialize(uint16_t bufferSize, uint16_t dataSize, void *data) {
	pCircularBuffer_t buffer = malloc(sizeof (CircularBuffer));
	if (buffer != NULL) {
		buffer->dataSize = dataSize;
		buffer->bufferSize = bufferSize; // * dataSize;
		buffer->tail = 0u;
		buffer->head = 0u;

		if (data == NULL) {
			buffer->base = malloc(bufferSize * dataSize); //calloc(bufferSize, dataSize);
		} else {
			buffer->base = data;
		}
		if (buffer->base == NULL) //memory full
		{
			free(buffer);
			buffer = NULL;
		}
	}
	return (pCircularBuffer_t) (buffer);
}

void Cb_destroy(pCircularBuffer_t buffer) {
	free(buffer->base);
	free(buffer);
}

_Bool Cb_put(pCircularBuffer_t buffer, void *item) {
	if ((buffer == NULL) || Cb_full(buffer)) {
		return false; // Buffer is full or not initialized
	}
	memcpy(Cb_ends_at(buffer), item, buffer->dataSize);
	Cb_commit_write(buffer);
	return true;
}

_Bool Cb_get(pCircularBuffer_t buffer, void *item) {
	if ((buffer == NULL) || Cb_empty(buffer)) {
		return false; // Empty buffer or not initialized
	}
	memcpy(item, Cb_starts_at(buffer), buffer->dataSize);
	Cb_commit_read(buffer);
	return true;
}

_Bool Cb_peek(pCircularBuffer_t buffer, void *item, uint16_t offset) {
	if ((buffer == NULL) || Cb_empty(buffer) || (Cb_available_data(buffer) < offset)) {
		return false; // Empty buffer or not initialized
	}
	memcpy(item, (buffer->base + (((buffer->tail + offset) % buffer->bufferSize) * buffer->dataSize)), buffer->dataSize);
	return true;
}

_Bool Cb_flush(pCircularBuffer_t buffer) {
	if ((buffer == NULL)) { //memory not initialized
		return false;
	}
	buffer->tail = 0u;
	buffer->head = 0u;

	return true;
}

inline _Bool CbIsFull(pCircularBuffer_t buffer) {
	return (_Bool) Cb_full(buffer);
}

inline _Bool CbIsEmpty(pCircularBuffer_t buffer) {
	return (_Bool)Cb_empty(buffer);
}

inline uint16_t CbAvailableData(pCircularBuffer_t buffer) {
	return (uint16_t) Cb_available_data(buffer);
}