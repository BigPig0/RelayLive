/*!
 * \file cstl_easy.c
 * \date 2018/12/13 14:02
 *
 * \author wlla
 * Contact: user@company.com
 *
 * \brief 
 *
 * TODO: libcstl中迭代器操作的封装，使插入、查找操作更加简单
 *
 * \note
*/

#include "cstl_easy.h"

/** common */
void string_map_compare(const void* cpv_first, const void* cpv_second, void* pv_output) {
    *(bool_t*)pv_output = string_less(*(const string_t**)cpv_first, *(const string_t**)cpv_second) /*? true : false*/;
}

void string_map_hash(const void* cpv_input, void* pv_output)
{
    pair_t*  ppair_pair = NULL;
    string_t* key_value = NULL;
    size_t   t_sum = 0;
    size_t   i = 0;
    size_t   t_len = 0;

    assert(cpv_input != NULL);
    assert(pv_output != NULL);

    ppair_pair = (pair_t*)cpv_input;
    key_value = *(string_t**)pair_first(ppair_pair);
    t_len = string_size(key_value);


    for (i = 0; i < t_len; ++i) {
        t_sum += (size_t)string_at(key_value, i)[0];
    }

    *(size_t*)pv_output = t_sum;
}

void not_free_int(int n){

}

/** map_t */
void map_insert_easy(map_t* pmap_map, const void* key, const void* value) {
    pair_t* pt_pair = create_pair(void*, void*);
    pair_init_elem(pt_pair, key, value);
    map_insert(pmap_map, pt_pair);
    pair_destroy(pt_pair);
}

void* map_find_easy_str(map_t* pmap_map, const char* key) {
    string_t *str_key = create_string();
    map_iterator_t fit;
    map_iterator_t end = map_end(pmap_map);
    pair_t* pt_pair;
    void* ret = NULL;
    string_init_cstr(str_key, key);
    fit = map_find(pmap_map, str_key);
    if (iterator_not_equal(fit, end)) {
        pt_pair = (pair_t*)iterator_get_pointer(fit);
        ret = *(void**)pair_second(pt_pair);
    }
    return ret;
}

/** hash_map_t */
void hash_map_insert_easy(hash_map_t* phmap_map, const void* key, const void* value) {
    pair_t* pt_pair = create_pair(void*, void*);
    pair_init_elem(pt_pair, key, value);
    hash_map_insert(phmap_map, pt_pair);
    pair_destroy(pt_pair);
}

void* hash_map_find_easy_str(hash_map_t* phmap_map, const char* key) {
    string_t *str_key = create_string();
    hash_map_iterator_t fit;
    hash_map_iterator_t end = hash_map_end(phmap_map);
    pair_t* pt_pair;
    void* ret = NULL;
    string_init_cstr(str_key, key);
    fit = hash_map_find(phmap_map, str_key);
    if (iterator_not_equal(fit, end)) {
        pt_pair = (pair_t*)iterator_get_pointer(fit);
        ret = *(void**)pair_second(pt_pair);
    }
    return ret;
}