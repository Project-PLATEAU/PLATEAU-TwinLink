#pragma once

template<class Enum>
class EnumT
{
public:
    using EnumType = typename Enum::Type;
    using ValueType = typename Enum::ValueType;
    using ConstructorArgType = typename Enum::Type;

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
    bool IsValid() const { return Val < Enum::Max; }

    static EnumType Max() { return Enum::Max; }

private:
    EnumType Val = Enum::Undefined;
};
