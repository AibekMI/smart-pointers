#pragma once

#include <type_traits>
#include <utility>

template <class V>
inline constexpr bool kIsCompressed = std::is_empty_v<V> && !std::is_final_v<V>;

template <class V, class U>
inline constexpr bool kCouldBeCompressedToFirst = kIsCompressed<U> && !std::is_base_of_v<U, V>;

// Me think, why waste time write lot code, when few code do trick.
template <typename F, typename S,
          bool could_be_compressed_to_first = kCouldBeCompressedToFirst<F, S>,
          bool could_be_compressed_to_second = kCouldBeCompressedToFirst<S, F>>
class CompressedPair {
public:
    template <typename FF, typename SS>
    constexpr CompressedPair(FF&& first, SS&& second)
        : first_(std::forward<FF>(first)), second_(std::forward<SS>(second)){};

    CompressedPair() : CompressedPair(F(), S()){};

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    S& GetSecond() {
        return second_;
    }

    const S& GetSecond() const {
        return second_;
    };

private:
    F first_;
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, true, false> : S {
public:
    template <typename FF, typename SS>
    constexpr CompressedPair(FF&& first, SS&& second)
        : S(std::forward<SS>(second)), first_(std::forward<FF>(first)){};

    CompressedPair() : CompressedPair(F(), S()){};

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    S& GetSecond() {
        return *static_cast<S* const>(this);
    }

    const S& GetSecond() const {
        return *static_cast<const S* const>(this);
    };

private:
    F first_;
};

template <typename F, typename S>
class CompressedPair<F, S, true, true> : F, S {
public:
    template <typename FF, typename SS>
    constexpr CompressedPair(FF&& first, SS&& second)
        : F(std::forward<FF>(first)), S(std::forward<SS>(second)){};

    CompressedPair() : CompressedPair(F(), S()){};

    F& GetFirst() {
        return *static_cast<F* const>(this);
    }

    const F& GetFirst() const {
        return *static_cast<const F* const>(this);
    }

    S& GetSecond() {
        return *static_cast<S* const>(this);
    }

    const S& GetSecond() const {
        return *static_cast<const S* const>(this);
    };
};

template <typename F, typename S>
class CompressedPair<F, S, false, true> : F {
public:
    template <typename FF, typename SS>
    constexpr CompressedPair(FF&& first, SS&& second)
        : F(std::forward<F>(first)), second_(std::forward<S>(second)){};

    CompressedPair() : CompressedPair(F(), S()){};

    F& GetFirst() {
        return *static_cast<F* const>(this);
    }

    const F& GetFirst() const {
        return *static_cast<const F* const>(this);
    }

    S& GetSecond() {
        return second_;
    }

    const S& GetSecond() const {
        return second_;
    };

private:
    S second_;
};
