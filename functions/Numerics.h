/* -*- mode: c++; indent-tabs-mode: nil; tab-width: 2 -*-
 *
 *  $Id: Numerics.h,v 1.1 2006/10/09 07:07:59 Paul Pedersen Exp $
 *
 *	Copyright 2006-2007 FLWOR Foundation.
 *  Authors: John Cowan, Paul Pedersen
 */

#ifndef XQP_NUMERICS_H
#define XQP_NUMERICS_H

#include "context/common.h"
#include "functions/function.h"
#include "runtime/item_iterator.h"
#include "types/sequence_type.h"
#include "util/rchandle.h"

#include <vector>

namespace xqp {

class zorba;

/*______________________________________________________________________
|  
| 6.2 Operators on Numeric Values
|_______________________________________________________________________*/


// 6.2.1 op:numeric-add
// --------------------
class op_numeric_add : public function
{
public:
	op_numeric_add() {}
	op_numeric_add(const signature&);
	~op_numeric_add() {}

public:
	Iterator_t operator()(const yy::location& loc, std::vector<Iterator_t>&) const;
	sequence_type_t type_check(signature&) const;
	bool validate_args(std::vector<Iterator_t>&) const;
};

class op_numeric_add_int : public function
{
public:
	op_numeric_add_int(const signature&);
	~op_numeric_add_int() {}

public:
	Iterator_t operator()(const yy::location&, std::vector<Iterator_t>&) const;
	sequence_type_t type_check(signature&) const;
	bool validate_args(std::vector<Iterator_t>&) const;
};


// 6.2.2 op:numeric-subtract
// -------------------------
class op_numeric_subtract : public function
{
public:
	op_numeric_subtract(const signature&);
	~op_numeric_subtract() {}

public:
	Iterator_t operator()(const yy::location& loc, std::vector<Iterator_t>&) const;
	sequence_type_t type_check(signature&) const;
	bool validate_args(std::vector<Iterator_t>&) const;
};


// 6.2.3 op:numeric-multiply
// -------------------------
class op_numeric_multiply : public function
{
public:
	op_numeric_multiply(const signature&);
	~op_numeric_multiply() {}

public:
	Iterator_t operator()(const yy::location& loc, std::vector<Iterator_t>&) const;
	sequence_type_t type_check(signature&) const;
	bool validate_args(std::vector<Iterator_t>&) const;
};


// 6.2.4 op:numeric-divide
// -----------------------
class op_numeric_divide : public function
{
public:
	op_numeric_divide(const signature&);
	~op_numeric_divide() {}

public:
	Iterator_t operator()(const yy::location& loc, std::vector<Iterator_t>&) const;
	sequence_type_t type_check(signature&) const;
	bool validate_args(std::vector<Iterator_t>&) const;
};

// 6.2.5 op:numeric-integer-divide
// -------------------------------
class op_numeric_integer_divide : public function
{
public:
	op_numeric_integer_divide(const signature&);
	~op_numeric_integer_divide() {}

public:
	Iterator_t operator()(const yy::location& loc, std::vector<Iterator_t>&) const;
	sequence_type_t type_check(signature&) const;
	bool validate_args(std::vector<Iterator_t>&) const;
};

// 6.2.6 op:numeric-mod
// --------------------
class op_numeric_mod : public function
{
public:
	op_numeric_mod(const signature&);
	~op_numeric_mod() {}

public:
	Iterator_t operator()(const yy::location& loc, std::vector<Iterator_t>&) const;
	sequence_type_t type_check(signature&) const;
	bool validate_args(std::vector<Iterator_t>&) const;
};

// 6.2.7 op:numeric-unary-plus
// ---------------------------
class op_numeric_unary_plus : public function
{
public:
	op_numeric_unary_plus(const signature&);
	~op_numeric_unary_plus() {}

public:
	Iterator_t operator()(const yy::location& loc, std::vector<Iterator_t>&) const;
	sequence_type_t type_check(signature&) const;
	bool validate_args(std::vector<Iterator_t>&) const;
};

// 6.2.8 op:numeric-unary-minus
// ----------------------------
class op_numeric_unary_minus : public function
{
public:
	op_numeric_unary_minus(const signature&);
	~op_numeric_unary_minus() {}

public:
	Iterator_t operator()(const yy::location& loc, std::vector<Iterator_t>&) const;
	sequence_type_t type_check(signature&) const;
	bool validate_args(std::vector<Iterator_t>&) const;
};



/*______________________________________________________________________
|  
| 6.3 Comparison Operators on Numeric Values
|_______________________________________________________________________*/

// 6.3.1 op:numeric-equal
//-----------------------
// 6.3.2 op:numeric-less-than
// 6.3.3 op:numeric-greater-than


/*______________________________________________________________________
|  
| 6.4 Functions on Numeric Values
|_______________________________________________________________________*/

// 6.4.1 fn:abs
class fn_abs : public function 
{
public:
	fn_abs(const signature&);
	~fn_abs() {}

public:
	Iterator_t operator()( const yy::location& loc, std::vector<Iterator_t>&) const;
	sequence_type_t type_check(signature&) const;
	bool validate_args(std::vector<Iterator_t>&) const;
};

// 6.4.2 fn:ceiling
// 6.4.3 fn:floor
// 6.4.4 fn:round
// 6.4.5 fn:round-half-to-even


}	/* namespace xqp */
#endif	/* XQP_NUMERICS_H */
