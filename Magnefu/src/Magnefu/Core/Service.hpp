#pragma once


namespace Magnefu 
{

    struct Service 
    {

        virtual void                init(void* configuration) { }
        virtual void                shutdown() { }

    }; 

#define MF_DECLARE_SERVICE(Type)    static Type* instance();

} 