//
//  IntVec2.cpp
//  Util/Base
//
//  Created by shamaton.
//
//

#include "IntVec2.h"


IntVec2::IntVec2()
: x(0), y(0)
{
}

IntVec2::IntVec2(int xx, int yy)
: x(xx), y(yy)
{
}

IntVec2::IntVec2(const IntVec2& p1, const IntVec2& p2)
{
    set(p1, p2);
}

IntVec2::IntVec2(const IntVec2& copy)
{
    set(copy);
}

IntVec2::~IntVec2()
{
}


void IntVec2::set(int xx, int yy)
{
    this->x = xx;
    this->y = yy;
}

void IntVec2::set(const IntVec2& v)
{
    this->x = v.x;
    this->y = v.y;
}

void IntVec2::set(const IntVec2& p1, const IntVec2& p2)
{
    x = p2.x - p1.x;
    y = p2.y - p1.y;
}


void IntVec2::setPoint(int xx, int yy)
{
    this->x = xx;
    this->y = yy;
}

bool IntVec2::equals(const IntVec2& target) const
{
    return ((this->x == target.x) && (this->y == target.y));
}

void IntVec2::negate()
{
    x = -x;
    y = -y;
}

void IntVec2::add(const IntVec2& v)
{
    x += v.x;
    y += v.y;
}

void IntVec2::subtract(const IntVec2& v)
{
    x -= v.x;
    y -= v.y;
}

void IntVec2::scale(int scalar)
{
    x *= scalar;
    y *= scalar;
}

void IntVec2::scale(const IntVec2& scale)
{
    x *= scale.x;
    y *= scale.y;
}

//---------------------------------------------------------
// oprator
//---------------------------------------------------------
const IntVec2 IntVec2::operator+(const IntVec2& v) const
{
    IntVec2 result(*this);
    result.add(v);
    return result;
}

IntVec2& IntVec2::operator+=(const IntVec2& v)
{
    add(v);
    return *this;
}

const IntVec2 IntVec2::operator-(const IntVec2& v) const
{
    IntVec2 result(*this);
    result.subtract(v);
    return result;
}

IntVec2& IntVec2::operator-=(const IntVec2& v)
{
    subtract(v);
    return *this;
}

const IntVec2 IntVec2::operator-() const
{
    IntVec2 result(*this);
    result.negate();
    return result;
}

const IntVec2 IntVec2::operator*(int s) const
{
    IntVec2 result(*this);
    result.scale(s);
    return result;
}

IntVec2& IntVec2::operator*=(int s)
{
    scale(s);
    return *this;
}

const IntVec2 IntVec2::operator/(const int s) const
{
    return IntVec2(this->x / s, this->y / s);
}

bool IntVec2::operator<(const IntVec2& v) const
{
    if (x == v.x)
    {
        return y < v.y;
    }
    return x < v.x;
}

bool IntVec2::operator==(const IntVec2& v) const
{
    return x == v.x && y == v.y;
}

bool IntVec2::operator!=(const IntVec2& v) const
{
    return x != v.x || y != v.y;
}

const IntVec2 operator*(float x, const IntVec2& v)
{
    IntVec2 result(v);
    result.scale(x);
    return result;
}
