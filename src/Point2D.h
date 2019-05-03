/**
 *  Point2D.h - Defines a 2-D point class 
 *
 *  Part of the VisExLib (the Visual Experiment Library)
 *
 *  Copyright (C) 2012-2014 by Melchi M. Michel <melchi.michel@rutgers.edu>
 *
 *  Licensed under GNU General Public License 3.0 or later. 
 *  Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 */
#pragma once

#pragma warning(disable:4267)
#pragma warning(disable:4244)

#define _USE_MATH_DEFINES
#include <math.h>
#include <complex>
#include <vector>
#include <iomanip>
#include <fstream>

#ifndef SQR
#define SQR(A) ((A)*(A))
#endif

// Point2D class

struct Point2D{
	double x;
	double y;

private:
	double sign(double x){
		double result = (x<0)? -1.0:1.0;
		return result;
	}
public:

	// Member methods
	Point2D rotate(double theta,bool in_degrees=true){
		theta = in_degrees? theta*M_PI/180.0:theta;
		return Point2D(x*cos(theta)-y*sin(theta),x*sin(theta)+y*cos(theta));
	}
	double vlength(){
		return sqrt(SQR(x)+SQR(y));
	}
	Point2D normalize(){
		double norm = vlength();
		return Point2D(x/norm,y/norm);
	}
	double angle(bool in_degrees = true){
		// Returns angular distance from +y axis
		double rho = sqrt(SQR(x)+SQR(y));
		double theta;
		if(rho==0.0){
			theta = 0.0;
		}else{
			if((x==0)&&(y>=0)){
				theta = 0;
			}else{
				theta = acos(-y/rho)*sign(x)+M_PI;
			}
		}
		theta = in_degrees? theta*180/M_PI:theta;
		return theta;
	}
	void print(void){
		printf("\nPoint2D([%f, %f])\n",x,y);
	}

	// Overloaded operators
	friend Point2D &operator-(Point2D &lhs){
		lhs.x = -lhs.x;
		lhs.y = -lhs.y;
		return lhs;
	}
	friend Point2D operator+(Point2D lhs,Point2D rhs){
		return Point2D(lhs.x+rhs.x,lhs.y+rhs.y);
	}
	Point2D& operator+=(Point2D &rhs){
		this->x+=rhs.x;
		this->y+=rhs.y;
		return *this;
	}
	friend Point2D operator+(Point2D lhs,double rhs){
		return Point2D(lhs.x+rhs,lhs.y+rhs);
	}
	friend Point2D operator-(Point2D lhs,double rhs){
		return Point2D(lhs.x-rhs,lhs.y-rhs);
	}
	friend Point2D operator-(Point2D lhs,Point2D rhs){
		return Point2D(lhs.x-rhs.x,lhs.y-rhs.y);
	}
	Point2D& operator-=(Point2D &rhs){
		this->x-=rhs.x;
		this->y-=rhs.y;
		return *this;
	}
	friend Point2D operator*(Point2D lhs,double rhs){
		return Point2D(lhs.x*rhs,lhs.y*rhs);
	}
	Point2D& operator*=(double &rhs){
		this->x*=rhs;
		this->y*=rhs;
		return *this;
	}
	friend Point2D operator/(Point2D lhs,double rhs){
		return Point2D(lhs.x/rhs,lhs.y/rhs);
	}
	Point2D& operator/=(double &rhs){
		this->x/=rhs;
		this->y/=rhs;
		return *this;
	}
	friend Point2D operator*(double lhs,Point2D rhs){
		return Point2D(lhs*rhs.x,lhs*rhs.y);
	}
	friend std::ifstream &operator>>(std::ifstream &fs, Point2D &rhs){
		fs.read(reinterpret_cast<char*>(&rhs.x),sizeof(double));
		fs.read(reinterpret_cast<char*>(&rhs.y),sizeof(double));
		return fs;
	}
	friend std::ostream &operator<<(std::ostream &ss, const Point2D &rhs){
		//ss.precision(4);
		ss<<std::fixed<<std::setprecision(2)<<"[ "<<rhs.x<<", "<<rhs.y<<"]";
		return ss;
	}

	// Constructors
	Point2D(double xin, double yin): x(xin),y(yin){}
	Point2D(double *in_ptr): x(in_ptr[0]),y(in_ptr[1]){}
	Point2D():x(0),y(0){}
};
