/*
 * tc.h
 *
 *  Created on: 25/02/2021
 *      Author: user1
 */

#ifndef TC_H_
#define TC_H_

typedef struct FakeTc
{

} TC_t;

TC_t TCC0;

void tc_enable(void * pTc);
void tc_set_resolution(void * pTc, unsigned char r);
unsigned long tc_get_resolution(void * pTc);
unsigned short tc_read_count(void * pTc);

#endif /* TC_H_ */
