#pragma once


namespace Magnefu 
{

    struct Service 
    {

        virtual void                Init(void* configuration) { }
        virtual void                Shutdown() { }

    }; 

#define MF_DECLARE_SERVICE(Type)    static Type* Instance();

} 