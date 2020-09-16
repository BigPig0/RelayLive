/*
 *  The private interface of string_t
 *  Copyright (C)  2008 - 2012  Wangbo
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  Author e-mail: activesys.wb@gmail.com
 *                 activesys@sina.com.cn
 */

#ifndef _CSTL_STRING_PRIVATE_H_
#define _CSTL_STRING_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

/** include section **/

/** constant declaration and macro section **/

/** data type declaration and struct, union, enum section **/
typedef basic_string_t string_t;

/** exported global variable declaration section **/

/** exported function prototype section **/
/**
 * Create string container auxiliary function.
 * @param pstr_string     uncreated container.
 * @return if create string successfully return true, otherwise return false.
 * @remarks if pstr_string == NULL, then the behavior is undefine.
 */
extern bool_t _create_string_auxiliary(string_t* pstr_string);

/**
 * Destroy string container auxiliary function.
 * @param pstr_string  string container.
 * @return void.
 * @remarks if pstr_string == NULL or string is not created by create_string() function, then the behavior
 *          is undefined. string container must be create_string, otherwise the behavior is undefined.
 */
extern void _string_destroy_auxiliary(string_t* pstr_string);

#ifdef __cplusplus
}
#endif

#endif /* _CSTL_STRING_PRIVATE_H_ */
/** eof **/

