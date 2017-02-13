/*
 * hapstik.h
 *
 * Created: 06.02.2017 23:23:42
 *  Author: tmueller
 */ 


#ifndef HAPSTIK_H_
#define HAPSTIK_H_

#include <Arduino.h>

class HAPStik {
	public:
		HAPStik();
		~HAPStik();
		void setPositionX(int position);
		void setPositionY(int position);
	};
	
	


#endif /* HAPSTIK_H_ */