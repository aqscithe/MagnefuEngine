#pragma once

#include "Magnefu/Core/Maths/Vectors.h"
#include "Magnefu/Core/Maths/Matrices.h"

#include <string>
#include <vector>
#include <variant>
#include <unordered_map>
#include <typeindex>
#include <xhash>


namespace Magnefu
{
    enum class UniformTypes
    {
        NONE,
        INT,
        INT_PTR,
        FLOAT,
        VEC2,
        VEC3,
        VEC4,
        MAT4
    };

    const std::unordered_map<std::type_index, UniformTypes> UniformTypeHashcodeMap =
    {
        {typeid(int),         UniformTypes::INT},
        {typeid(int*),        UniformTypes::INT_PTR},
        {typeid(float),       UniformTypes::FLOAT},
        {typeid(Maths::vec2), UniformTypes::VEC2},
        {typeid(Maths::vec3), UniformTypes::VEC3},
        {typeid(Maths::vec4), UniformTypes::VEC4},
        {typeid(Maths::mat4), UniformTypes::MAT4}
    };

    class Uniform {
    public:
        Uniform() : 
            m_Name("NONE"), m_TypeIndex(typeid(int)), m_Value(0)
        {}
        Uniform(const std::string& name, const std::type_index& type)
            : m_Name(name), m_TypeIndex(type) {}

        template <typename T>
        void SetValue(const T& value) {
            m_Value = value;
        }

        template <typename T>
        void SetValue(const T* value) {
            m_Value = *value;
        }

        template <typename T>
        T& GetValue() {
            return std::get<T>(m_Value);
        }

        const std::string& GetName() const {
            return m_Name;
        }

        const std::type_index& GetType() const {
            return m_TypeIndex;
        }

    private:
        std::string m_Name;
        //std::size_t m_HashCode;
        std::type_index m_TypeIndex;
        std::variant<int, int*, float, Maths::mat4, Maths::vec4, Maths::vec3, Maths::vec2> m_Value; // Support int, float, and vec3 values
    };
}