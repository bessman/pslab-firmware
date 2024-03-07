#ifndef LOGICANALYZER_H
#define LOGICANALYZER_H

#include "../commands.h"

/**
 * @brief Capture logic level changes on pins LA1-4
 *
 * @details
 * Every time the logic level changes on an active pin, a timestamp is copied
 * to the sample buffer.
 *
 * Three types of logic level change can be captured: any, falling, or rising.
 *
 * One pin may be chosen as trigger. If so, capture only starts once an edge is
 * detected on that pin. Otherwise, capture begins immediately.
 *
 * @param uint8_t num_channels
 * Value from 1 to 4, determining how many of the LA pins to use.
 * @param uint16_t events
 * Number of edges to capture per channel.
 * @param Edge edge
 * @param Channel trigger
 *
 * @return SUCCESS
 */
response_t LA_capture(void);

/**
 * @brief Stop capture
 *
 * @details
 * Stop edge capture ahead of time, and release associated resourses.
 *
 * @return SUCCESS
 */
response_t LA_stop(void);

/**
 * @brief Get states of LA1-4 immediately before capture started
 *
 * @details
 * Low nibble of returned byte correspond to pins LA1-4, one pin per bit.
 *
 * @return SUCCESS
 */
response_t LA_get_initial_states(void);

#endif /* LOGICANALYZER_H */
