#pragma once

template<class T>
class FTwinLinkEnumT {
public:
    using FTwinLinkEnumType = T;
    using ValueType = std::underlying_type_t<T>;
    using ConstructorArgType = T;

    FTwinLinkEnumT() {}
    constexpr FTwinLinkEnumT(FTwinLinkEnumType V) : Val(V) {}

    ValueType GetValue() const { return static_cast<ValueType>(Val); };
    FTwinLinkEnumType GetEnumValue() const { return Val; }

    bool operator == (FTwinLinkEnumType V) const {
        return Val == V;
    }

    bool operator != (FTwinLinkEnumType V) const {
        return Val != V;
    }

    bool operator == (FTwinLinkEnumT V) {
        return Val == V.Val;
    }

    bool operator != (FTwinLinkEnumT V) {
        return Val != V.Val;
    }

    template<class U>
    inline bool operator == (FTwinLinkEnumT<U> V) const = delete;
    template<class U>
    inline bool operator != (FTwinLinkEnumT<U> V) const = delete;

    // 有効な値かどうか
    bool IsValid() const { return Val < T::Max; }

    static FTwinLinkEnumType Max() { return T::Max; }

private:
    FTwinLinkEnumType Val = T::Undefined;
};
