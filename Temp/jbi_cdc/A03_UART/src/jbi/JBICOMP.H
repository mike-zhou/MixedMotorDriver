/****************************************************************************/
/*																			*/
/*	Module:			jbicomp.h												*/
/*																			*/
/*					Copyright (C) Altera Corporation 1997-2001				*/
/*																			*/
/*	Description:	Contains the function prototypes for compressing		*/
/*					and uncompressing Boolean array data.					*/
/*																			*/
/****************************************************************************/

#ifndef INC_JBICOMP_H
#define INC_JBICOMP_H

unsigned long jbi_uncompress
(
	unsigned long in_address, 
	unsigned long in_length, 
	unsigned long out_address, 
	unsigned long out_length,
	int version
);


#endif /* INC_JBICOMP_H */
