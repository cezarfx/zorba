/*
 * Copyright 2006-2008 The FLWOR Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstring>
#include <vector>

#include "zorbaerrors/error_manager.h"
#include "zorbatypes/zorbatypesError.h"

#include "ascii_util.h"
#include "regex.h"
#include "stl_util.h"

using namespace std;
U_NAMESPACE_USE

#define bs_c "\\p{L}\\d.:\\p{M}-"       /* \c equivalent contents */
#define bs_i "\\p{L}_:"                 /* \i equivalent contents */

namespace zorba {

///////////////////////////////////////////////////////////////////////////////

typedef uint32_t icu_flags_t;

static icu_flags_t convert_xquery_flags( char const *xq_flags ) {
  icu_flags_t icu_flags = 0;
  if ( xq_flags ) {
    for ( char const *f = xq_flags; *f; ++f ) {
      switch ( *f ) {
        case 'i': icu_flags |= UREGEX_CASE_INSENSITIVE; break;
        case 'm': icu_flags |= UREGEX_MULTILINE       ; break;
        case 's': icu_flags |= UREGEX_DOTALL          ; break;
        case 'x': icu_flags |= UREGEX_COMMENTS        ; break;
        default:
          throw zorbatypesException( xq_flags, ZorbatypesError::FORX0001 );
      }
    }
  }
  return icu_flags;
}

void convert_xquery_re( zstring const &xq_re, zstring *icu_re,
                        char const *xq_flags ) {
  bool const x_flag = xq_flags ? ::strchr( xq_flags, 'x' ) : false;

  icu_re->clear();
  icu_re->reserve( xq_re.length() );    // approximate

  bool got_backslash = false;
  bool in_char_class = false;           // within [...]

  bool in_backref = false;              // '\'[1-9][0-9]*
  unsigned backref_no = 0;
  vector<bool> cap_sub;                 // true = open; false = closed

  FOR_EACH( zstring, xq_c, xq_re ) {
    if ( got_backslash ) {
      if ( x_flag && !in_char_class && ascii::is_space( *xq_c ) ) {
        //
        // XQuery F&O: 7.6.1.1: If [the 'x' flag is] present, whitespace
        // characters ... in the regular expression are removed prior to
        // matching with one exception: whitespace characters within character
        // class expressions ... are not removed.
        //
        continue;
      }
      got_backslash = false;
      switch ( *xq_c ) {
        case 'c': // NameChar
          *icu_re += "[" bs_c "]";
          continue;
        case 'C': // ^\c
          *icu_re += "[^" bs_c "]";
          continue;
        case 'i': // initial NameChar
          *icu_re += "[" bs_i "]";
          continue;
        case 'I': // ^\i
          *icu_re += "[^" bs_i "]";
          continue;
        default:
          if ( ascii::is_digit( *xq_c ) ) {
            backref_no = *xq_c - '0';
            if ( !backref_no )          // \0 is illegal
              ZORBA_ERROR( FORX0002 );
            if ( in_char_class ) {
              //
              // XQuery F&O 7.6.1: Within a character class expression,
              // \ followed by a digit is invalid.
              //
              ZORBA_ERROR( FORX0002 );
            }
            in_backref = true;
          }
          *icu_re += '\\';
          break;
      }
    } else {
      if ( in_backref ) {
        //
        // XQuery F&O 7.6.1: The construct \N where N is a single digit is
        // always recognized as a back-reference; if this is followed by
        // further digits, these digits are taken to be part of the back-
        // reference if and only if the resulting number NN is such that the
        // back-reference is preceded by NN or more unescaped opening
        // parentheses.
        //
        if ( cap_sub.size() > 9 && ascii::is_digit( *xq_c ) )
          backref_no = backref_no * 10 + (*xq_c - '0');
        else
          in_backref = false;
        //
        // XQuery F&O 7.6.1: The regular expression is invalid if a back-
        // reference refers to a subexpression that does not exist or whose
        // closing right parenthesis occurs after the back-reference.
        //
        if ( backref_no > cap_sub.size() )
          ZORBA_ERROR( FORX0002 );
        if ( cap_sub[ backref_no - 1 ] )
          ZORBA_ERROR( FORX0002 );
      }
      switch ( *xq_c ) {
        case '\\':
          got_backslash = true;
          continue;
        case '(':
          cap_sub.push_back( true );
          break;
        case ')': {
          if ( cap_sub.empty() )
            ZORBA_ERROR( FORX0002 );
          if ( !cap_sub.back() )
            ZORBA_ERROR( FORX0002 );
          cap_sub.back() = false;
          break;
        }
        case '[':
          in_char_class = true;
          break;
        case ']':
          in_char_class = false;
          break;
        default:
          if ( x_flag && ascii::is_space( *xq_c ) ) {
            if ( !in_char_class )
              continue;
            //
            // This is similar to the above case for removing whitespace except
            // ICU removes *all* whitespace (even within character classes)
            // unless the whitespace is escaped; hence we have to escape it.
            //
            *icu_re += '\\';
          }
      }
    }
    *icu_re += *xq_c;
  } // FOR_EACH

  //
  // XML Schema Part 2 F.1.1: [Unicode Database] groups code points into a
  // number of blocks such as Basic Latin (i.e., ASCII), Latin-1 Supplement,
  // Hangul Jamo, CJK Compatibility, etc. The set containing all characters
  // that have block name X (with all white space stripped out), can be
  // identified with a block escape \p{IsX}.
  //
  // However, ICU uses \p{InX} rather than \p{IsX}.  Also note that the '5'
  // below is correct since "\\" represents a single '\'.
  //
  ascii::replace_all( *icu_re, "\\p{Is", 5, "\\p{In", 5 );
}

///////////////////////////////////////////////////////////////////////////////

#ifndef ZORBA_NO_UNICODE

namespace unicode {

bool regex::compile( string const &pattern, char const *flags ) {
  UErrorCode status = U_ZERO_ERROR;
  delete matcher_;
  matcher_ = new RegexMatcher( pattern, convert_xquery_flags( flags ), status );
  if ( U_FAILURE( status ) ) {
    delete matcher_;
    matcher_ = 0;
    return false;
  }
  return true;
}

bool regex::match_part( string const &s ) {
  assert( matcher_ );
  matcher_->reset( s );
  return matcher_->find() != 0;
}

bool regex::match_whole( string const &s ) {
  assert( matcher_ );
  matcher_->reset( s );
  UErrorCode status = U_ZERO_ERROR;
  return matcher_->matches( status ) && U_SUCCESS( status );
}

bool regex::next( re_type_t re_type, string const &s, size_type *pos,
                  string *substring, bool *matched ) {
  assert( matcher_ );
  assert( pos );
  unicode::size_type const s_len = s.length();
  if ( *pos < s_len ) {
    matcher_->reset( s );
    UErrorCode status = U_ZERO_ERROR;
    if ( matcher_->find( *pos, status ) ) {
      size_type const end = matcher_->end( status );
      if ( substring ) {
        size_type const start = matcher_->start( status );
        switch ( re_type ) {
          case re_is_match:
            substring->setTo( s, start, end - start );
            break;
          case re_is_separator:
            substring->setTo( s, *pos, start - *pos );
            break;
        }
      }
      *pos = end;
      if ( matched )
        *matched = true;
      return true;
    }
    if ( re_type == re_is_separator ) {
      //
      // Special case: the RE did not match starting at pos, but there *is* a
      // last token.  For example, given the RE of "," and:
      //
      //    s   = "a,b,c"
      //    pos =  01234
      //
      // then calling next_token() with a pos of 4 will not match another ","
      // but we should return the final token after the last "," (the "c").
      //
      if ( substring )
        substring->setTo( s, *pos, s_len - *pos );
      *pos = s_len;
      if ( matched )
        *matched = false;
      return true;
    }
  }
  if ( matched )
    *matched = false;
  return false;
}

bool regex::replace_all( string const &in, string const &replacement,
                         string *out ) {
  assert( matcher_ );
  assert( out );
  matcher_->reset( in );
  UErrorCode status = U_ZERO_ERROR;
  *out = matcher_->replaceAll( replacement, status );
  return U_SUCCESS( status ) == TRUE;
}

bool regex::replace_all( char const *in, char const *replacement,
                         string *out ) {
  string u_in, u_replacement;
  return  to_string( in, &u_in ) &&
          to_string( replacement, &u_replacement ) &&
          replace_all( u_in, u_replacement, out );
}

} // namespace unicode

#endif /* ZORBA_NO_UNICODE */

///////////////////////////////////////////////////////////////////////////////

} // namespace zorba
/* vim:set et sw=2 ts=2: */
