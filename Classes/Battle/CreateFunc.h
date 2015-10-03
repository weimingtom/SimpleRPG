//
//  CreateFunc.h
//  Choukyu
//
//  Created by shamaton.
//
//

#ifndef Choukyu_CreateFunc_h
#define Choukyu_CreateFunc_h

#include<utility>
template<class Derived>
struct create_func {
    template<class... Args>
    static Derived* create(Args&&... args) {
        auto p = new Derived();
        if (p->init(std::forward<Args>(args)...)) {
            p->autorelease();
            return p;
        } else {
            delete p;
            return nullptr;
        }
    }
};

#endif
