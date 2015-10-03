//
//  IntVec2.h
//  Util/Base
//
//  Created by shamaton.
//
//

#ifndef __Util_Base__IntVec2__
#define __Util_Base__IntVec2__

#include <algorithm>
#include <functional>
#include <math.h>
#include "math/CCMathBase.h"

/**
 * Defines a 2-element int point vector.
 */
class IntVec2
{
public:
    
    /**
     * The x coordinate.
     */
    int x;
    
    /**
     * The y coordinate.
     */
    int y;
    
    IntVec2();
    IntVec2(int xx, int yy);
    IntVec2(const IntVec2& p1, const IntVec2& p2);
    IntVec2(const IntVec2& copy);
    
    /**
     * Destructor.
     */
    ~IntVec2();
    
    void set(int xx, int yy);
    void set(const int* array);
    void set(const IntVec2& v);
    void set(const IntVec2& p1, const IntVec2& p2);
    
    void negate();
    
    void add(const IntVec2& v);
    
    void scale(int scalar);
    void scale(const IntVec2& scale);
    
    void subtract(const IntVec2& v);
    
    
    const IntVec2 operator+(const IntVec2& v) const;
    IntVec2& operator+=(const IntVec2& v);
    const IntVec2 operator-(const IntVec2& v) const;
    IntVec2& operator-=(const IntVec2& v);
    const IntVec2 operator-() const;
    const IntVec2 operator*(int s) const;
    IntVec2& operator*=(int s);
    const IntVec2 operator/(int s) const;
    bool operator<(const IntVec2& v) const;
    bool operator==(const IntVec2& v) const;
    bool operator!=(const IntVec2& v) const;
    
    //code added compatible for Point
public:
    /**
     * @js NA
     * @lua NA
     */
    void setPoint(int xx, int yy);
    /**
     * @js NA
     */
    bool equals(const IntVec2& target) const;

};


#endif /* defined(__Util_Base__IntVec2__) */
