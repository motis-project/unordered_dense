#pragma once

#include "ankerl/unordered_dense.h"

#include "cista/equal_to.h"
#include "cista/hashing.h"
#include "cista/serialization.h"

#ifndef cista_member_offset
#    define cista_member_offset(s, m) (static_cast<cista::offset_t>(offsetof(s, m)))
#endif

template <class Key,
          class T, // when void, treat it as a set.
          class Hash,
          class KeyEqual,
          class AllocatorOrContainer,
          class Bucket,
          bool IsSegmented>
struct serializable_table
    : ankerl::unordered_dense::detail::table<Key, T, Hash, KeyEqual, AllocatorOrContainer, Bucket, IsSegmented> {
    template <typename Ctx>
    friend void serialize(Ctx& c, serializable_table const* origin, cista::offset_t const pos) {
        using Type = std::decay_t<decltype(*origin)>;

        auto const size = cista::serialized_size<Bucket>() * origin->m_num_buckets;
        auto const start = origin->m_buckets == nullptr ? cista::NULLPTR_OFFSET
                                                        : c.write(origin->m_buckets, size, std::alignment_of_v<Bucket>);

        c.write(pos + cista_member_offset(Type, m_buckets),
                cista::convert_endian<Ctx::MODE>(
                    start == cista::NULLPTR_OFFSET ? start : start - cista_member_offset(Type, m_buckets) - pos));
        c.write(pos + cista_member_offset(Type, m_num_buckets), cista::convert_endian<Ctx::MODE>(origin->m_num_buckets));
        c.write(pos + cista_member_offset(Type, m_max_bucket_capacity),
                cista::convert_endian<Ctx::MODE>(origin->m_max_bucket_capacity));
        c.write(pos + cista_member_offset(Type, m_max_load_factor),
                cista::convert_endian<Ctx::MODE>(origin->m_max_load_factor));
        c.write(pos + cista_member_offset(Type, m_shifts), cista::convert_endian<Ctx::MODE>(origin->m_shifts));

        cista::serialize(c, &origin->m_values, pos + cista_member_offset(Type, m_values));

        if (origin->m_buckets != nullptr) {
            auto it = start;
            for (auto i = 0U; i != origin->m_num_buckets; it += cista::serialized_size<Bucket>(), ++i) {
                serialize(c, &origin->m_buckets[i], it);
            }
        }
    }

    template <typename Ctx>
    friend void check_state(Ctx const& c, serializable_table* el) {
        c.check_ptr(el->m_buckets, cista::checked_multiplication(static_cast<std::size_t>(el->m_num_buckets), sizeof(Bucket)));
    }

    template <typename Ctx>
    friend void convert_endian_and_ptr(Ctx const& c, serializable_table* el) {
        deserialize(c, &el->m_buckets);
        c.convert_endian(el->m_num_buckets);
        c.convert_endian(el->m_max_bucket_capacity);
        c.convert_endian(el->m_max_load_factor);
        c.convert_endian(el->m_shifts);
    }

    template <typename Ctx, typename Fn>
    friend void recurse(Ctx&, serializable_table* el, Fn&& fn) {
        fn(&el->m_values);
        for (auto i = 0U; i != el->m_num_buckets; ++i) {
            fn(&el->m_buckets[i]);
        }
    }

    template <std::size_t NMaxTypes>
    friend constexpr auto static_type_hash(serializable_table const*, cista::hash_data<NMaxTypes> h) noexcept {
        using cista::static_type_hash;
        h = h.combine(cista::hash("ankerl::table"));
        h = static_type_hash(cista::null<Key>(), h);
        h = static_type_hash(cista::null<T>(), h);
        h = static_type_hash(cista::null<AllocatorOrContainer>(), h);
        h = static_type_hash(cista::null<Bucket>(), h);
        h = h.combine(IsSegmented);
        return h;
    }
};

namespace cista {

struct hash_all {
    using is_transparent = void;

    template <typename T>
    auto operator()(T const& t) const noexcept {
        return hashing<T>{}(t);
    }
};

struct equals_all {
    using is_transparent = void;

    template <typename A, typename B>
    auto operator()(A const& a, B const& b) const noexcept {
        return equal_to<A>{}(a, b);
    }
};

namespace offset {

template <class Key,
          class T,
          class Hash = hash_all,
          class KeyEqual = equals_all,
          class AllocatorOrContainer = cista::offset::vector<std::pair<Key, T>>,
          class Bucket = ankerl::unordered_dense::bucket_type::standard>
using ankerl_map = serializable_table<Key, T, Hash, KeyEqual, AllocatorOrContainer, Bucket, false>;

template <class Key,
          class Hash = hash_all,
          class KeyEqual = equals_all,
          class AllocatorOrContainer = cista::offset::vector<Key>,
          class Bucket = ankerl::unordered_dense::bucket_type::standard>
using ankerl_set = serializable_table<Key, void, Hash, KeyEqual, AllocatorOrContainer, Bucket, false>;

} // namespace offset

namespace raw {

template <class Key,
          class T,
          class Hash = hash_all,
          class KeyEqual = equals_all,
          class AllocatorOrContainer = cista::raw::vector<std::pair<Key, T>>,
          class Bucket = ankerl::unordered_dense::bucket_type::standard>
using ankerl_map = serializable_table<Key, T, Hash, KeyEqual, AllocatorOrContainer, Bucket, false>;

template <class Key,
          class Hash = hash_all,
          class KeyEqual = equals_all,
          class AllocatorOrContainer = cista::raw::vector<Key>,
          class Bucket = ankerl::unordered_dense::bucket_type::standard>
using ankerl_set = serializable_table<Key, void, Hash, KeyEqual, AllocatorOrContainer, Bucket, false>;

} // namespace raw

} // namespace cista

#undef cista_member_offset