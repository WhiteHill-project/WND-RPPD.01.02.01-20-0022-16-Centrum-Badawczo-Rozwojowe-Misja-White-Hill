#pragma once

#include <stdint.h>
#include <stdbool.h>

/** Initialize this structure with the Cb_initialize function! */
typedef struct _CircularBuffer{
	volatile uint16_t tail; /** Start index */
	volatile uint16_t head; /** End index */
	uint16_t bufferSize; /** Size of the buffer in bytes */
	uint16_t dataSize; /** Size of one element */
	void *base; /** Pointer to the beginning of the data */
} CircularBuffer;

typedef CircularBuffer *pCircularBuffer_t;

/** Initializes a circular buffer.
 *  @param bufferSize The number of elements the buffer should store (the usable elements will be bufferSize-1)
 *  @param dataSize The size of one element in bytes.
 *  @param data A pointer to the freed space in memory where the buffer should be place, if NULL memory will be allocated.
 *  @return return Pointer to the initialized buffer or NULL
 */
pCircularBuffer_t Cb_initialize(uint16_t bufferSize, uint16_t dataSize, void *data);

/** Destroy a circular buffer. 
 *  @param buffer Target buffer.
 */
void Cb_destroy(pCircularBuffer_t buffer);

/** Puts one element to the circular buffer
 *  @param buffer Target buffer.
 *  @param item   Pointer to the element to add, only the data of the element will be used.
 *  @return Returns false if the buffer is full and true if successful.
 */
_Bool Cb_put(pCircularBuffer_t buffer, void *item);
/** Get one element from the circular buffer.
 *  @param buffer Target buffer.
 *  @param item Pointer to the element where the data should be stored.
 *  @return Returns false if the buffer is empty and true if successful.
 */
_Bool Cb_get(pCircularBuffer_t buffer, void *item);

/** Read one element from the circular buffer without get it.
 *  @param buffer Target buffer.
 *  @param item Pointer to the element where the data should be stored.
 *  @param 
 *  @return Returns false if the buffer is empty or offset is bigger than available data and true if successful.
 */
_Bool Cb_peek(pCircularBuffer_t buffer, void *item, uint16_t offset);

/** Flushes the circular buffer.
 *  @param buffer Target buffer.
 *  @return Returns false if the buffer is not initialize true if successful.
 */
_Bool Cb_flush(pCircularBuffer_t buffer);

/** Returns circular buffer full status
 *  @param buffer Target buffer.
 *  @return true if the buffer is full.
 */
inline _Bool CbIsFull(pCircularBuffer_t buffer);

/** Returns circular buffer empty status
 *  @param buffer Target buffer.
 *  @return true if the buffer is empty.
 */
inline _Bool CbIsEmpty(pCircularBuffer_t buffer);

/** Returns circular buffer available data
 *  @param buffer Target buffer.
 *  @return available free elements in circural buffer.
 */
inline uint16_t CbAvailableData(pCircularBuffer_t buffer);

/**
 * @}
 */

