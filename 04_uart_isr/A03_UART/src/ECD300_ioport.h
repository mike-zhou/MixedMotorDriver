/*
 * ECD300_ioport.h
 *
 * Created: 2014/12/18 19:18:19
 *  Author: user
 */ 


#ifndef ECD300_IOPORT_H_
#define ECD300_IOPORT_H_

#define ecd300SetPortData(port, data) (port.OUT)=(data)
#define ecd300GetPortData(port) (port.OUT)

#endif /* ECD300_IOPORT_H_ */