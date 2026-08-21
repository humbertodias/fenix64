/****************************************************************************/
/*                                                                          */
/* Fenix - Videogame compiler/interpreter                                   */
/* Current release       : PROJECT 1.0 - 0.84                               */
/* Last stable release   :                                                                          */
/* Project documentation : http://fenix.divsite.net                         */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* This program is free software; you can redistribute it and/or modify     */
/* it under the terms of the GNU General Public License as published by     */
/* the Free Software Foundation; either version 2 of the License, or        */
/* (at your option) any later version.                                      */
/*                                                                          */
/* This program is distributed in the hope that it will be useful,          */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/* GNU General Public License for more details.                             */
/*                                                                          */
/* You should have received a copy of the GNU General Public License        */
/* along with this program; if not, write to the Free Software              */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA */
/*                                                                          */
/****************************************************************************/
/* Copyright © 1999 José Luis Cebrián Pagüe                                 */
/* Copyright © 2002 FENIX PROJECT 1.0 TEAM                                  */
/****************************************************************************/

/****************************************************************************/
/* FILE        : strings.c                                                  */
/* DESCRIPTION : Strings management. Includes any function related to       */
/*               variable-length strings. Those strings are allocated       */
/*               in dynamic memory with reference counting.                 */
/****************************************************************************/
/* HISTORY     : 29/01/2001 (jlceb) the pointer/reference/dontfree arrays   */
/*                  are now of dynamic size. Some comments written.         */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif


#include "fxi.h"
#include "dcb.h"

/****************************************************************************/
/* GLOBAL VARIABLES :                                                       */
/****************************************************************************/

/* 1 to debug string operations to the console (lots of text) */
int report_string = 0 ;
/*
#undef gr_con_printf
#define gr_con_printf(...) printf(__VA_ARGS__);fflush(stdout);
*/
/****************************************************************************/
/* STATIC VARIABLES :                                                       */
/****************************************************************************/

/* Fixed string memory. The DCB fixed strings are stored here */
static char   * string_mem = 0 ;
static int      string_allocated = 0 ;
static int      string_used = 0 ;

/* Pointers to each string's text. Every string is allocated using strdup()
   or malloc(). A pointer of a unused slot is 0. Exception: "fixed" strings
   are stored in a separate memory block and should not be freed */
static char  ** string_ptr ;

/* Usage count for each string. An unused slot has a count of 0 */
static int    * string_uct ;

/* Fixed-memory flag: 1 for "fixed" strings */
static char   * string_dontfree ;

/* How many strings slots are used. This is only the bigger id in use + 1.
   There may be unused slots in this many positions */
static int      string_count = 1 ;

/* How many string slots are available in the ptr, uct and dontfree arrays */
static int      string_ptr_allocated = 0 ;

/****************************************************************************/
/* FUNCTION : string_init                                                   */
/****************************************************************************/
/* Allocate memory for the dynamic arrays. You should call this function    */
/* before anything else in this file. There is enough space for about       */
/* 1024 short strings, that should be enough for simple programs. More      */
/* space is allocated as needed.                                            */
/****************************************************************************/

void string_init ()
{
    string_mem = (char *) malloc (4096) ;
    string_allocated = 4096 ;
    string_used = 0 ;

    string_ptr_allocated = 1024 ;
    string_ptr = (char **) malloc (1024 * sizeof(char *)) ;
    string_uct = (int *) malloc (1024 * sizeof(int)) ;
    string_dontfree = (char *) malloc (1024 * sizeof(char)) ;

    /* Create an empty string with ID 0 */
    string_mem[0] = 0 ;
    string_ptr[0] = string_mem ;

    string_count = 1 ;
}

/****************************************************************************/
/* FUNCTION : string_alloc                                                  */
/****************************************************************************/
/* int bytes: how many new strings we could need                            */
/****************************************************************************/
/* Increase the size of the internal string arrays. This limits how many    */
/* strings you can have in memory at the same time, and this should be      */
/* called when every identifier slot available is already used.             */
/****************************************************************************/

static void string_alloc (int count)
{
    string_ptr_allocated += count ;

    string_ptr = (char **) realloc (string_ptr, string_ptr_allocated * sizeof(char *)) ;
    string_uct = (int *) realloc (string_uct, string_ptr_allocated * sizeof(int)) ;
    string_dontfree = (char *) realloc (string_dontfree, string_ptr_allocated * sizeof(char)) ;

    if (!string_ptr || !string_uct || !string_dontfree)
        gr_error ("string_alloc: sin memoria\n") ;
}

/****************************************************************************/
/* FUNCTION : string_dump                                                   */
/****************************************************************************/
/* Shows all the strings in memory in the console, including the reference  */
/* count (usage count) of each string.                                      */
/****************************************************************************/

void string_dump ()
{
    int i ;
    int used=0;

    gr_con_printf ("[STRING] ---- Dumping MaxID=%d strings ----\n", string_count) ;

    for (i = 0 ; i < string_count ; i++){
        if (string_ptr[i]){
            if (string_uct[i] == 0){
                if (!string_dontfree[i]){
                    free(string_ptr[i]) ;
                    string_ptr[i] = NULL ; // Splinter
                    continue ;
                }
            }
            used++;
        } else {
            continue ;
        }
        gr_con_printf ("[STRING] %4d %1d [%4d]: {%s}\n", i, string_uct[i], string_dontfree[i], string_ptr[i]) ;
    }
    gr_con_printf ("[STRING] ---- Dumping Used=%d End ----\n", used) ;
}

/****************************************************************************/
/* FUNCTION : string_get                                                    */
/****************************************************************************/
/* int code: identifier of the string you want                              */
/****************************************************************************/
/* Returns the contens of an string. Beware: this pointer with only be      */
/* valid while no other string function is called.                          */
/****************************************************************************/

const char * string_get (int code)
{
    assert (code < string_count && code >= 0) ;
    if (report_string){
        gr_con_printf ("[STRING] string_get %d\n", code) ;
    }
    return string_ptr[code] ;
}

/****************************************************************************/
/* FUNCTION : string_load                                                   */
/****************************************************************************/
/* file * fp: the DCB file (must be opened)                                 */
/*                                                                          */
/* This function uses the global "dcb" struct. It should be already filled. */
/****************************************************************************/
/* Loads the string portion of a DCB file. This includes an area with all   */
/* the text (that will be stored in the string_mem pointer) and an array of */
/* the offsets of every string. This function fills the internal arrayswith */
/* all this data and allocates memory if needed.                            */
/****************************************************************************/

void string_load (file * fp)
{
    int * string_offset, n;

    string_count = dcb.data.NStrings ;
    string_used  = dcb.data.SText ;
    file_seek (fp, dcb.data.OStrings, SEEK_SET) ;
    string_offset = (int *) malloc (4 * string_count) ;
    if (!string_offset) {
        gr_error ("string_load: not enough memory\n") ;
    }
    file_read (fp, string_offset, 4 * string_count) ;
    if (string_used > string_allocated)
    {
        string_allocated = string_used ;
        string_mem = (char *) realloc (string_mem, string_used) ;
    }
    if (string_count + 128 > string_ptr_allocated) {
        string_alloc (string_count + 512 - string_ptr_allocated) ;
    }

    file_seek (fp, dcb.data.OText, SEEK_SET) ;
    file_read (fp, string_mem, string_used) ;

    for (n = 0 ; n < string_count ; n++)
    {
        string_ptr[n]       = string_mem + string_offset[n] ;
        string_uct[n]       = 0 /*25*/ ; // -- Fix Splinter
        string_dontfree[n]  = 1 ;
    }

    string_ptr[n]       = NULL ;
    string_uct[n]       = 0 ; // -- Fix Splinter

    free (string_offset) ;
}

/****************************************************************************/
/* FUNCTION : string_use                                                    */
/****************************************************************************/
/* int code: identifier of the string you are using                         */
/****************************************************************************/
/* Increase the usage counter of an string. Use this when you store the     */
/* identifier of the string somewhere.                                      */
/****************************************************************************/

void string_use (int code)
{
    if (code < 0 || code > string_count || !string_ptr[code]) {
        return;
    }

    string_uct[code]++ ;
    if (report_string) {
        gr_con_printf ("[STRING] String %d used (count: %d)\n", code, string_uct[code]) ;
    }
}

/****************************************************************************/
/* FUNCTION : string_discard                                                */
/****************************************************************************/
/* int code: identifier of the string you don't need anymore                */
/****************************************************************************/
/* Decrease the usage counter of an string. Use this when you retrieve the  */
/* identifier of the string and discard it, or some memory (like private    */
/* variables) containing the string identifier is destroyed. If the usage   */
/* count is decreased to zero, the string will be discarted, and the        */
/* identifier may be used in the future by other string.                    */
/****************************************************************************/

void string_discard (int code)
{
    if ( code < 0 || code > string_count || !string_ptr[code]) {
        return;
    }

    if (string_uct[code] < 1)
    {
        if (report_string) {
            gr_con_printf ("[STRING] string_discard: String %d released but already discarted\n", code) ;
        }
        return ;
    }

    string_uct[code]-- ;

    if (report_string) {
        gr_con_printf ("[STRING] string_discard: String %d released (count: %d)\n", code, string_uct[code]) ;
    }

    if ( string_uct[code] < 1 )
    {
        if (report_string) {
            gr_con_printf ("[STRING] string_discard: String %d released and discarted\n", code) ;
        }

        if (!string_dontfree[code]) {
            free(string_ptr[code]) ;
            string_ptr[code] = NULL ; // Splinter
            string_uct[code] = 0 ;
        }

        if (report_string ) {
            if ( string_dontfree[code]) {
                gr_con_printf ("[STRING] string_discard: (Memory don't freed - %d is special string, count: %d)\n", code, string_uct[code]) ;
            } else {
                gr_con_printf ("[STRING] string_discard: String %d released and discarted (count: %d)\n", code, string_uct[code]) ;
            }
        }
    }
}

/****************************************************************************/
/* FUNCTION : string_coalesce                                               */
/****************************************************************************/
/* Does some garbage collection (frees any memory used by unreferenced      */
/* strings). This should not be necessary, because strings are now discarted*/
/* inmediately when its usage counts is zero.                               */
/****************************************************************************/

void string_coalesce()
{
    int n ;

    if (string_count < string_ptr_allocated/2)
        return ;

    for (n = 1 ; n < string_ptr_allocated ; n++)
    {
        if (!string_uct[n])
        {
            if (!string_dontfree[n]) {
                free (string_ptr[n]) ;
                string_ptr[n] = NULL ; // Splinter
            }
        }
    }
}

/****************************************************************************/
/* FUNCTION : string_getid                                                  */
/****************************************************************************/
/* Searchs for an available ID and returns it. If none available, more space*/
/* is allocated for the new string. This is used for new strings only.      */
/****************************************************************************/

static int string_getid ()
{
    int n ;

    // Si tengo suficientes alocados, retorno el siguiente segun string_count
    if (string_count < string_ptr_allocated) {
        return string_count++ ;
    }

    // Ya no tengo mas espacio, entonces busco alguno libre
    for (n = 1 ; n < string_ptr_allocated ; n++) {
        if (!string_ptr[n]) {
            return n ;
        }
    }
    // Incremento espacio
    string_alloc (1024) ;
    gr_con_printf ("[STRING] \xAC" "12*PANIC\xAC" "7 Too many strings, allocating more space") ;

    // Devuelvo el string_count + 1, ya que ahora tengo 1024 mas que antes
    return string_count++ ;
}

/****************************************************************************/
/* FUNCTION : string_new                                                    */
/****************************************************************************/
/* Create a new string. It returns its ID. Note that it uses strdup()       */
/* TODO: do something if no memory available                                */
/****************************************************************************/

int string_new (const char * ptr)
{
    char * str = strdup(ptr) ;
    int    id ;

    assert (str) ;
    id = string_getid() ;

    if (report_string) {
        gr_con_printf ("[STRING] String %d created: \"%s\"\n", id, str) ;
    }

    string_ptr[id] = str ;
    string_uct[id] = 0 ;
    string_dontfree[id] = 0 ;
    return id ;
}

/*
 *  FUNCTION : string_newa
 *
 *  Create a new string from a text buffer section
 *
 *  PARAMS:
 *              ptr         Pointer to the text buffer at start position
 *              count       Number of characters
 *
 *  RETURN VALUE:
 *      ID of the new string
 */

int string_newa (const char * ptr, unsigned count)
{
    char * str = malloc(count+1);
    int    id ;

    assert (str) ;
    id = string_getid() ;

    strncpy (str, ptr, count);
    str[count] = 0;
    string_ptr[id] = str ;
    string_uct[id] = 0 ;
    string_dontfree[id] = 0 ;

    if (report_string){
        gr_con_printf ("[STRING] (newa) String %d created: \"%s\"\n", id, str) ;
    }

    return id ;
}

/****************************************************************************/
/* FUNCTION : string_concat                                                 */
/****************************************************************************/
/* Add some text to an string and return the resulting string. This does not*/
/* modify the original string, but creates a new one.                       */
/****************************************************************************/

int string_concat (int code1, char * str2)
{
    char * str1 ;

    assert (code1 < string_count && code1 >= 0) ;

    str1 = string_ptr[code1] ;
    assert (str1) ;

    str1 = (char *) realloc(str1, strlen(str1) + strlen(str2) + 1) ;
    assert (str1) ;

    strcat (str1, str2) ;

    string_ptr[code1] = str1 ;
    return code1 ;
}

/****************************************************************************/
/* FUNCTION : string_add                                                    */
/****************************************************************************/
/* Add an string to another one and return the resulting string. This does  */
/* not modify the original string, but creates a new one.                   */
/****************************************************************************/

int string_add (int code1, int code2)
{
    const char * str1 = string_get(code1) ;
    const char * str2 = string_get(code2) ;
    char * str3 ;
    int id ;

    assert (str1) ;
    assert (str2) ;

    str3 = (char *) malloc(strlen(str1) + strlen(str2) + 1) ;
    assert (str3) ;

    strcpy (str3, str1) ;
    strcat (str3, str2) ;
    id = string_getid() ;

    string_ptr[id] = str3 ;
    string_uct[id] = 0 ;
    string_dontfree[id] = 0 ;

    if (report_string){
        gr_con_printf ("[STRING] (add) String %d created: \"%s\"\n", id, str3) ;
    }

    return id ;
}

/****************************************************************************/
/* FUNCTION : string_ptoa                                                   */
/****************************************************************************/
/* Convert a pointer to a new created string and return its ID.             */
/****************************************************************************/

int string_ptoa (void * n)
{
    char * str ;
    int id ;

    str = (char *) malloc(64) ;
    assert (str)  ;

    sprintf (str, "%p", n) ;

    id = string_getid() ;
    string_ptr[id] = str ;
    string_uct[id] = 0 ;
    string_dontfree[id] = 0 ;

    if (report_string){
        gr_con_printf ("[STRING] (ptoa) String %d created: \"%s\"\n", id, str) ;
    }

    return id ;
}

/****************************************************************************/
/* FUNCTION : string_ftoa                                                   */
/****************************************************************************/
/* Convert a float to a new created string and return its ID.               */
/****************************************************************************/

int string_ftoa (float n)
{
    char * str, * ptr ;
    int id ;

    str = (char *) malloc(64) ;
    assert (str)  ;

    sprintf (str, "%f", n) ;
    ptr = str + strlen(str) - 1 ;
    while (ptr >= str)
    {
        if (*ptr != '0') break ;
        *ptr-- = 0 ;
    }
    if (ptr >= str && *ptr == '.') *ptr = 0 ;
    if (*str == 0) strcpy (str, "0") ;

    id = string_getid() ;
    string_ptr[id] = str ;
    string_uct[id] = 0 ;
    string_dontfree[id] = 0 ;

    if (report_string){
        gr_con_printf ("[STRING] (ftoa) String %d created: \"%s\"\n", id, str) ;
    }

    return id ;
}

/****************************************************************************/
/* FUNCTION : string_itoa                                                   */
/****************************************************************************/
/* Convert an integer to a new created string and return its ID.            */
/****************************************************************************/

int string_itoa (int n)
{
    char * str ;
    int id ;

    str = (char *) malloc(32) ;
    assert (str)  ;

    sprintf (str, "%d", n) ;

    id = string_getid() ;
    string_ptr[id] = str ;
    string_uct[id] = 0 ;
    string_dontfree[id] = 0 ;

    if (report_string){
        gr_con_printf ("[STRING] (itoa) String %d created: \"%s\"\n", id, str) ;
    }

    return id ;
}

/****************************************************************************/
/* FUNCTION : string_uitoa                                                  */
/****************************************************************************/
/* Convert an unsigned integer to a new created string and return its ID.   */
/****************************************************************************/

int string_uitoa (unsigned int n)
{
    char * str ;
    int id ;

    str = (char *) malloc(32) ;
    assert (str)  ;

    sprintf (str, "%u", n) ;

    id = string_getid() ;
    string_ptr[id] = str ;
    string_uct[id] = 0 ;
    string_dontfree[id] = 0 ;

    if (report_string){
        gr_con_printf ("[STRING] (uitoa) String %d created: \"%s\"\n", id, str) ;
    }

    return id ;
}

/****************************************************************************/
/* FUNCTION : string_comp                                                   */
/****************************************************************************/
/* Compare two strings using strcmp and return the result                   */
/****************************************************************************/

int string_comp (int code1, int code2)
{
    const char * str1 = string_get(code1) ;
    const char * str2 = string_get(code2) ;

    return strcmp (str1, str2) ;
}

/****************************************************************************/
/* FUNCTION : string_char                                                   */
/****************************************************************************/
/* Extract a character from a string. The parameter nchar can be:           */
/* - 0 or positive: return this character from the left (0 = leftmost)      */
/* - negative: return this character from the right (-1 = rightmost)        */
/* The result is not the ASCII value, but the identifier of a new string    */
/* that is create in the process and contains only the extracted character  */
/****************************************************************************/

int string_char (int n, int nchar)
{
    const char * str = string_get(n) ;
    int          len ;
    char buffer[2] ;

    assert (str) ;
    len = strlen(str) ;

    if (nchar >= len)
        nchar = len ;

    if (nchar < 0)
        nchar = len + nchar ;

    if (nchar < 0)
        nchar = len ;

    buffer[0] = str[nchar] ;
    buffer[1] = 0 ;

    return string_new (buffer) ;
}

/****************************************************************************/
/* FUNCTION : string_substr                                                 */
/****************************************************************************/
/* Extract a substring from a string. The parameters can be:                */
/* - 0 or positive: count this character from the left (0 = leftmost)       */
/* - negative: count this character from the right (-1 = rightmost)         */
/*                                                                          */
/* NO MORE: If first > last, the two values are swapped before returning the result  */
/****************************************************************************/

int string_substr (int code, int first, int len)
{
    const char * str = string_get(code) ;
    char       * ptr ;
    int          rlen, n ;

    assert (str) ;
    rlen = strlen(str) ;

    if (first < 0) first = rlen + first ;
    if (first < 0) first = 0 ;
    if (first > (rlen - 1)) return string_new("") ;

    if (len < 0) {
        len = rlen + (len + 2) - first - 1 ;
        if (len < 1) return string_new("") ;
    }

    if ((first + len) > rlen) len = (rlen - first) ;

    ptr = (char *)malloc(len+1) ;
    memcpy (ptr, str + first, len) ;
    ptr[len]            = 0 ;

    n                   = string_getid() ;
    string_ptr[n]       = ptr ;
    string_uct[n]       = 0 ;
    string_dontfree[n]  = 0 ;

    if (report_string){
        gr_con_printf ("[STRING] (substr) String %d created: \"%s\"\n", n, ptr) ;
    }

    return n ;
}


/*
 *  FUNCTION : string_find
 *
 *  Find a substring. Returns the position of the leftmost character (0
 *  for the leftmost position) or -1 if the string was not found.
 *
 *  PARAMS:
 *              code1                   Code of the string
 *              code2                   Code of the substring
 *              first                   Character to start the search
 *                                      (negative to search backwards)
 *
 *  RETURN VALUE:
 *      Result of the comparison
 */

int string_find (int code1, int code2, int first)
{
    const char * str1 = string_get(code1) ;
    const char * str2 = string_get(code2) ;
    int pos, len1, len ;

    assert (str1 && str2) ;
    len1 = strlen(str1);
    len = strlen(str2) ;

    pos = first;
    if (pos < 0)
    {
        pos = len1 + pos;
        if (pos < 0)
            pos = 0;
    }
    if (pos > len1)
        pos = len1;

    for ( ; str1[pos] && pos >= 0 ; first >= 0 ? pos++ : pos--)
    {
        if (memcmp(str1+pos, str2, len) == 0)
            return pos ;
    }
    return -1 ;
}

/*
 *  FUNCTION : string_ucase
 *
 *  Convert an string to upper case. It does not alter the given string, but
 *  creates a new string in the correct case and returns its id.
 *
 *  PARAMS:
 *              code                    Internal code of original string
 *
 *  RETURN VALUE:
 *      Code of the resulting string
 */

int string_ucase (int code)
{
    const char * str = string_get(code) ;
    char       * bptr, * ptr ;
    int          id ;

    assert (str) ;
    bptr = (char *)malloc(strlen(str)+1) ;
    assert (bptr) ;

    for (ptr = bptr; *str ; ptr++, str++)
        *ptr = TOUPPER(*str) ;

    *ptr = 0 ;
    id = string_getid() ;
    string_ptr[id] = bptr ;
    string_uct[id] = 0 ;
    string_dontfree[id] = 0 ;

    if (report_string){
        gr_con_printf ("[STRING] (ucase) String %d created: \"%s\"\n", id, bptr) ;
    }

    return id ;
}

/*
 *  FUNCTION : string_lcase
 *
 *  Convert an string to lower case. It does not alter the given string, but
 *  creates a new string in the correct case and returns its id.
 *
 *  PARAMS:
 *              code                    Internal code of original string
 *
 *  RETURN VALUE:
 *      Code of the resulting string
 */

int string_lcase (int code)
{
    const char * str = string_get(code) ;
    char       * bptr, * ptr ;
    int          id ;

    assert (str) ;
    bptr = (char *)malloc(strlen(str)+1) ;
    assert (bptr) ;

    for (ptr = bptr; *str ; ptr++, str++)
        *ptr = TOLOWER(*str) ;

    *ptr = 0 ;
    id = string_getid() ;
    string_ptr[id] = bptr ;
    string_uct[id] = 0 ;
    string_dontfree[id] = 0 ;

    if (report_string){
        gr_con_printf ("[STRING] (lcase) String %d created: \"%s\"\n", id, bptr) ;
    }

    return id ;
}

/*
 *  FUNCTION : string_strip
 *
 *  Create a copy of a string, without any leading or ending blanks
 *
 *  PARAMS:
 *              code                    Internal code of original string
 *
 *  RETURN VALUE:
 *      Code of the resulting string
 */

int string_strip (int code)
{
    const char * ptr = string_get(code) ;
    char *       ostr;
    char *       bptr;
    int          id = string_new(ptr);

    ostr = (char *)string_get(id) ;
    bptr = ostr;
    assert (bptr);

    while (*ptr == ' ' || *ptr == '\n' || *ptr == '\r' || *ptr == '\t')
        ptr++;

    while (*ptr)
        *bptr++ = *ptr++;

    while (bptr > ostr && (bptr[-1] == ' ' || bptr[-1] == '\n' || bptr[-1] == '\r' || bptr[-1] == '\t'))
        bptr--;

    *bptr = 0;

    return id ;
}

/*
 *  FUNCTION : string_format
 *
 *  Format a number using the given characters
 *
 *  PARAMS:
 *              number                  Number to format
 *              spec                    Format specification
 *
 *  RETURN VALUE:
 *      Code of the resulting string
 */

int string_format (double number, int dec, char point, char thousands)
{
    char buffer[128];
    char buffer2[256];
    char * s, * t;
    int c;

    int negative = 0;

    if (number < 0)
    {
        negative = 1;
//        number = -number;
    }

    sprintf (buffer, "%.*f", dec, number);
    s = buffer + strlen(buffer)-1;
    t = buffer2 + 127;
    *t-- = 0;
    if (strchr (buffer, '.'))
    {
        if (dec < 0)
        {
            while (*s == '0' && s >= buffer)
                s--;
            if (*s == '.')
                *s-- = 0;
        }
        if (strchr (buffer, '.'))
        {
            while (s >= buffer)
            {
                if (*s == '.')
                {
                    *t-- = point;
                    s--;
                    break;
                }
                *t-- = *s--;
            }
        }
    }
    c = 0;
    while (s >= buffer)
    {
        *t-- = *s-- ;
        if (c == 2 && s >= buffer)
        {
            *t-- = thousands ;
            c = 0;
        }
        else c++;
    }
    return string_new (t+1);
}

/*
 *  FUNCTION : string_casecmp
 *
 *  Compare two strings (case-insensitive version)
 *
 *  PARAMS:
 *              code1                   Code of the first string
 *              code2                   Code of the second string
 *
 *  RETURN VALUE:
 *      Result of the comparison
 */

int string_casecmp (int code1, int code2)
{
    const unsigned char * str1 = string_get(code1) ;
    const unsigned char * str2 = string_get(code2) ;

    while (*str1 || *str2)
    {
        if (TOUPPER(*str1) != TOUPPER(*str2))
            return TOUPPER(*str1) - TOUPPER(*str2);

        str1++;
        str2++;
    }

    return 0 ;
}

/*
 *  FUNCTION : string_pad
 *
 *  Add spaces to the left or right of a string
 *
 *  PARAMS:
 *              code                    Code of the string
 *              total                   Total length of the final string
 *              align                   0 = align to the right; 1 = align to the left
 *
 *  RETURN VALUE:
 *      Result of the comparison
 */

int string_pad (int code, int total, int align)
{
    const char * ptr = string_get(code);

    int    len;
    int    spaces = 0;
    int    id;
    char * str;

    assert(ptr);
    len = strlen(ptr);
    if (len < total)
        spaces = total - len;

    if (!spaces) return string_new(ptr) ;

    str = malloc(total+1);
    assert (str);

    if (!align)
    {
        memset (str, ' ', spaces);
        strcpy (str + spaces, ptr) ;
    }
    else
    {
        strcpy (str, ptr) ;
        memset (str + len, ' ', spaces) ;
        str[total] = 0;
    }

    id = string_getid() ;
    string_ptr[id] = str ;
    string_uct[id] = 0 ;
    string_dontfree[id] = 0 ;

    if (report_string){
        gr_con_printf ("[STRING] (pad) String %d created: \"%s\"\n", id, str) ;
    }

    return id ;
}
