#pragma once

template<class T>
class EnumT
{
public:
    using EnumType = T;
    using ValueType = std::underlying_type_t<T>;
    using ConstructorArgType = T;

    EnumT(){}
    constexpr EnumT(EnumType V) : Val(V){}

    ValueType GetValue() const { return static_cast<ValueType>(Val); };
    EnumType GetEnumValue() const { return Val; }

    bool operator == (EnumType V) const
    {
        return Val == V;
    }

    bool operator != (EnumType V) const {
        return Val != V;
    }

    bool operator == (EnumT V)
    {
        return Val == V.Val;
    }

    bool operator != (EnumT V)
    {
        return Val != V.Val;
    }

    template<class U>
    inline bool operator == (EnumT<U> V) const = delete;
    template<class U>
    inline bool operator != (EnumT<U> V) const = delete;

    // 有効な値かどうか
    bool IsValid() const { return Val < T::Max; }

    static EnumType Max() { return T::Max; }

private:
    EnumType Val = T::Undefined;
};
