#pragma once


namespace Magnefu
{
    template <typename T>
    struct Handle
    {
        uint32_t Index;  // Index into the resources vector in the Pool
        uint32_t Generation;  // Generation counter for the resource
    };
}