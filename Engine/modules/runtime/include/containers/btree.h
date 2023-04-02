#pragma once
#include "platform/memory.h"
#include <parallel_hashmap/btree.h>
namespace Cyber 
{
    template<class K, class V, class Eq = phmap::Less<K>, class Allocator = cyber_stl_allocator<phmap::Pair<const K, V>>>
    using btree_map = phmap::btree_map<K, V, Eq, Allocator>;

    template<class K, class V, class Eq = phmap::Less<K>, class Allocator = cyber_stl_allocator<phmap::Pair<const K, V>>>
    using btree_multimap = phmap::btree_multimap<K, V, Eq, Allocator>;

    template<class K, class Eq = phmap::Less<K>, class Allocator = cyber_stl_allocator<K>>
    using btree_set = phmap::btree_set<K, Eq, Allocator>;

    template<class K, class Eq = phmap::Less<K>, class Allocator = cyber_stl_allocator<K>>
    using btree_multiset = phmap::btree_multiset<K, Eq, Allocator>;
}