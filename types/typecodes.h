/* -*- mode: c++; indent-tabs-mode: nil -*-
 *
 *  $Id: TypeCodes.h,v 1.1 2006/10/09 07:07:59 Paul Pedersen Exp $
 *
 *	Copyright 2006-2007 FLWOR Foundation.
 *  Authors: John Cowan, Paul Pedersen
 */

// TODO: add emptySequence type?
// TODO: what about anyAtomicType?

#ifndef XQP_TYPECODES_H
#define XQP_TYPECODES_H

#include <stdint.h>

namespace xqp {

// The following defines are really internal to this file.

// Masks and shifts for the bitfields in a type code.
// The representations are at the little end
static const int OBJECT_MASK = 1 << 30 | 1 << 29 | 1 << 28;
static const int OBJECT_SHIFT = 28;
static const int ARITY_MASK = 1 << 27 | 1 << 26;
static const int ARITY_SHIFT = 26;
static const int NODE_MASK = 1 << 25 | 1 << 24 | 1 << 23 | 1 << 22;
static const int NODE_SHIFT = 22;
static const int NONATOMIC_MASK = 1 << 21;
static const int NONATOMIC_SHIFT = 21;
static const int TYPE_MASK = 1 << 20 | 1 << 19 | 1 << 18 | 1 << 17 | 1 << 16;
static const int TYPE_SHIFT = 16;
static const int SUB_MASK = 1 << 15 | 1 << 14;
static const int SUB_SHIFT = 14;
static const int SUB2_MASK = 1 << 13 | 1 << 12;
static const int SUB2_SHIFT = 12;
static const int SUB3_MASK = 1 << 11 | 1 << 10;
static const int SUB3_SHIFT = 10;
static const int SUB4_MASK = 1 << 9;
static const int SUB4_SHIFT = 9;
static const int SUB5_MASK = 1 << 8 | 1 << 7;
static const int SUB5_SHIFT = 7;
static const int SUB6_MASK = 1 << 6;
static const int SUB6_SHIFT = 6;
static const int REP_MASK = 1 << 5 | 1 << 4 | 1 << 3 | 1 << 2 | 1 << 1 | 1 << 0;
static const int REP_SHIFT = 0;
static const int FULLTYPE_MASK = TYPE_MASK|SUB_MASK|SUB2_MASK|SUB3_MASK|SUB4_MASK|SUB5_MASK|SUB6_MASK;

// OBJECT specifies the most basic object type (value or something else)
// A SLACK_OBJECT is one which fills out a block at the end of a file
static const int VALUE_OBJECT = 0 << OBJECT_SHIFT;
static const int REF_OBJECT = 1 << OBJECT_SHIFT;
static const int FT_OBJECT = 2 << OBJECT_SHIFT;
static const int UPDATE_OBJECT = 3 << OBJECT_SHIFT;
static const int EXCEPTION_OBJECT = 4 << OBJECT_SHIFT;
static const int SLACK_OBJECT = 5 << OBJECT_SHIFT;

// ARITY specifies whether this value is an item or a sequence.
// Data items use ITEM_ARITY or SEQUENCE_ARITY.
// The other codes are used only to specify argument and result types of functions.
static const int ITEM_ARITY = 0 << ARITY_SHIFT;
static const int SEQUENCE_ARITY = 1 << ARITY_SHIFT;
static const int NONEMPTY_SEQ_ARITY = 2 << ARITY_SHIFT;
static const int OPT_ITEM_ARITY = 3 << ARITY_SHIFT;

// NODE specifies the node type.
// NOT_NODE is a dummy for values,
// ANY_NODE is a dummy for sequences of any type of node (but not a value).
static const int NOT_NODE = 0 << NODE_SHIFT;
static const int DOCUMENT_NODE = 1 << NODE_SHIFT;
static const int ELEMENT_NODE = 2 << NODE_SHIFT;
static const int ATTRIBUTE_NODE = 3 << NODE_SHIFT;
static const int NAMESPACE_NODE = 4 << NODE_SHIFT;
static const int PI_NODE = 5 << NODE_SHIFT;
static const int COMMENT_NODE = 6 << NODE_SHIFT;
static const int TEXT_NODE = 7 << NODE_SHIFT;
static const int ANY_NODE = 8 << NODE_SHIFT;

// NONATOMIC specifies if the type is atomic or not (complex or sequence)
// TODO: actually set this bit everywhere
// TODO: emptySequence definition
static const int NOT_NONATOMIC = 0 << NONATOMIC_SHIFT;
static const int IS_NONATOMIC = 1 << NONATOMIC_SHIFT;

// TYPE specifies the XML Schema type, plus these extensions:
// UNTYPED_TYPE is used for elements, UNTYPED_ATOMIC_TYPE for other node types.
// COMPLEX_TYPE is reserved for later use with complex types.
// EMPTY_TYPE is a pseudo-type used to construct empty sequences.
// ANY_TYPE is a pseudo-type used to specify "sequences of any type"
// RAW_BINARY_TYPE is not in XML Schema, and represents non-XML entity bodies.
static const int UNTYPED_TYPE = 0 << TYPE_SHIFT;
static const int UNTYPED_ATOMIC_TYPE = 1 << TYPE_SHIFT;
static const int STRING_TYPE = 2 << TYPE_SHIFT;
static const int DECIMAL_TYPE = 3 << TYPE_SHIFT;
static const int BOOLEAN_TYPE = 4 << TYPE_SHIFT;
static const int FLOAT_TYPE = 5 << TYPE_SHIFT;
static const int DOUBLE_TYPE = 6 << TYPE_SHIFT;
static const int DURATION_TYPE = 7 << TYPE_SHIFT;
static const int DATE_TIME_TYPE = 8 << TYPE_SHIFT;
static const int TIME_TYPE = 9 << TYPE_SHIFT;
static const int DATE_TYPE = 10 << TYPE_SHIFT;
static const int G_YEAR_MONTH_TYPE = 11 << TYPE_SHIFT;
static const int G_YEAR_TYPE = 12 << TYPE_SHIFT;
static const int G_MONTH_DAY_TYPE = 13 << TYPE_SHIFT;
static const int G_DAY_TYPE = 14 << TYPE_SHIFT;
static const int G_MONTH_TYPE = 15 << TYPE_SHIFT;
static const int HEX_BINARY_TYPE = 16 << TYPE_SHIFT;
static const int BASE_64_BINARY_TYPE = 17 << TYPE_SHIFT;
static const int ANY_URI_TYPE = 18 << TYPE_SHIFT;
static const int QNAME_TYPE = 19 << TYPE_SHIFT;
static const int NOTATION_TYPE = 20 << TYPE_SHIFT;
static const int COMPLEX_TYPE = 21 << TYPE_SHIFT;
static const int EMPTY_TYPE = 22 << TYPE_SHIFT;
static const int ANY_TYPE = 23 << TYPE_SHIFT;
static const int RAW_BINARY_TYPE = 24 << TYPE_SHIFT;

// SUB2-SUB5 specify derived XML Schema types.
static const int NORMALIZED_STRING_SUB = STRING_TYPE | 1 << SUB_SHIFT;
static const int TOKEN_SUB2 = NORMALIZED_STRING_SUB | 1 << SUB2_SHIFT;
static const int LANGUAGE_SUB3 = TOKEN_SUB2 | 1 << SUB3_SHIFT;
static const int NMTOKEN_SUB3 = TOKEN_SUB2 | 2 << SUB3_SHIFT;
static const int NAME_SUB3 = TOKEN_SUB2 | 3 << SUB3_SHIFT;
static const int NCNAME_SUB4 = NAME_SUB3 | 1 << SUB4_SHIFT;
static const int ID_SUB5 = NCNAME_SUB4 | 1 << SUB5_SHIFT;
static const int IDREF_SUB5 = NCNAME_SUB4 | 2 << SUB5_SHIFT;
static const int ENTITY_SUB5 = NCNAME_SUB4 | 3 << SUB5_SHIFT;

static const int INTEGER_SUB = DECIMAL_TYPE | 1 << SUB_SHIFT;
static const int NON_POSITIVE_INTEGER_SUB2 = INTEGER_SUB | 1 << SUB2_SHIFT;
static const int NEGATIVE_INTEGER_SUB3 = NON_POSITIVE_INTEGER_SUB2 | 1 << SUB3_SHIFT;
static const int LONG_SUB2 = INTEGER_SUB | 2 << SUB2_SHIFT;
static const int INT_SUB3 = LONG_SUB2 | 1 << SUB3_SHIFT;
static const int SHORT_SUB4 = INT_SUB3 | 1 << SUB4_SHIFT;
static const int BYTE_SUB5 = SHORT_SUB4 | 1 << SUB5_SHIFT;
static const int NON_NEGATIVE_INTEGER_SUB2 = INTEGER_SUB | 3 << SUB2_SHIFT;
static const int UNSIGNED_LONG_SUB3 = NON_NEGATIVE_INTEGER_SUB2 | 1 << SUB3_SHIFT;
static const int UNSIGNED_INT_SUB4 = UNSIGNED_LONG_SUB3 | 1 << SUB4_SHIFT;
static const int UNSIGNED_SHORT_SUB5 = UNSIGNED_INT_SUB4 | 1 << SUB5_SHIFT;
static const int UNSIGNED_BYTE_SUB6 = UNSIGNED_SHORT_SUB5 | 1 << SUB6_SHIFT;
static const int POSITIVE_INTEGER_SUB3 = NON_NEGATIVE_INTEGER_SUB2 | 2 << SUB3_SHIFT;

static const int DAY_TIME_DURATION_SUB = DURATION_TYPE | 1 << SUB_SHIFT;
static const int YEAR_MONTH_DURATION_SUB = DURATION_TYPE | 2 << SUB_SHIFT;

// REP specifies the representation (C++ class)
// More reps may be added in future.
// Initially DECIMAL_REP is not used.
// NO_REP means this type has no representation
// BARE_REP means this type does not need a special rep class but is just a type+length.
static const int NO_REP = 0 << REP_SHIFT;
static const int DOCUMENT_REP = 1 << REP_SHIFT;
static const int ELEMENT_REP = 2 << REP_SHIFT;
static const int ATTRIBUTE_REP = 3 << REP_SHIFT;
static const int NAMESPACE_REP = 4 << REP_SHIFT;
static const int PI_REP = 5 << REP_SHIFT;
static const int COMMENT_REP = 6 << REP_SHIFT;
static const int TEXT_REP = 7 << REP_SHIFT;
static const int STRING_REP = 8 << REP_SHIFT;
static const int DECIMAL_REP = 9 << REP_SHIFT;
static const int DOUBLE_REP = 10 << REP_SHIFT;
static const int BOOLEAN_REP = 11 << REP_SHIFT;
static const int TIME_REP = 12 << REP_SHIFT;
static const int BINARY_REP = 13 << REP_SHIFT;
static const int QNAME_REP = 14 << REP_SHIFT;
static const int SEQUENCE_REP = 15 << REP_SHIFT;
static const int REFERENCE_REP = 16 << REP_SHIFT;
static const int COMPLEX_TYPE_REP = 17 << REP_SHIFT;
static const int BARE_REP = 18 << REP_SHIFT;

// Okay, at last the bit-twiddling is done.  Now we just make an enum of type codes
// that assembles the correct values together.
// Use fix_typecodes to regenerate the Ref, Seq, SeqRef, Plus, and Opt definitions.

enum TypeCode {
	// Node items.
	documentNode = ITEM_ARITY + DOCUMENT_NODE + DOCUMENT_REP + UNTYPED_TYPE,
	documentNodeRef = REF_OBJECT + ITEM_ARITY + DOCUMENT_NODE + REFERENCE_REP + UNTYPED_TYPE,
	documentNodeSeq = SEQUENCE_ARITY + DOCUMENT_NODE + SEQUENCE_REP + UNTYPED_TYPE,
	documentNodeSeqRef = REF_OBJECT + SEQUENCE_ARITY + DOCUMENT_NODE + REFERENCE_REP + UNTYPED_TYPE,
	documentNodePlus = NONEMPTY_SEQ_ARITY + DOCUMENT_NODE + NO_REP + UNTYPED_TYPE,
	documentNodeOpt = OPT_ITEM_ARITY + DOCUMENT_NODE + NO_REP + UNTYPED_TYPE,
	elementNode = ITEM_ARITY + ELEMENT_NODE + ELEMENT_REP + UNTYPED_TYPE,
	elementNodeRef = REF_OBJECT + ITEM_ARITY + ELEMENT_NODE + REFERENCE_REP + UNTYPED_TYPE,
	elementNodeSeq = SEQUENCE_ARITY + ELEMENT_NODE + SEQUENCE_REP + UNTYPED_TYPE,
	elementNodeSeqRef = REF_OBJECT + SEQUENCE_ARITY + ELEMENT_NODE + REFERENCE_REP + UNTYPED_TYPE,
	elementNodePlus = NONEMPTY_SEQ_ARITY + ELEMENT_NODE + NO_REP + UNTYPED_TYPE,
	elementNodeOpt = OPT_ITEM_ARITY + ELEMENT_NODE + NO_REP + UNTYPED_TYPE,
	attributeNode = ITEM_ARITY + ATTRIBUTE_NODE + ATTRIBUTE_REP + UNTYPED_ATOMIC_TYPE,
	attributeNodeRef = REF_OBJECT + ITEM_ARITY + ATTRIBUTE_NODE + REFERENCE_REP + UNTYPED_ATOMIC_TYPE,
	attributeNodeSeq = SEQUENCE_ARITY + ATTRIBUTE_NODE + SEQUENCE_REP + UNTYPED_ATOMIC_TYPE,
	attributeNodeSeqRef = REF_OBJECT + SEQUENCE_ARITY + ATTRIBUTE_NODE + REFERENCE_REP + UNTYPED_ATOMIC_TYPE,
	attributeNodePlus = NONEMPTY_SEQ_ARITY + ATTRIBUTE_NODE + NO_REP + UNTYPED_ATOMIC_TYPE,
	attributeNodeOpt = OPT_ITEM_ARITY + ATTRIBUTE_NODE + NO_REP + UNTYPED_ATOMIC_TYPE,
	namespaceNode = ITEM_ARITY + NAMESPACE_NODE + NAMESPACE_REP + UNTYPED_TYPE,
	namespaceNodeRef = REF_OBJECT + ITEM_ARITY + NAMESPACE_NODE + REFERENCE_REP + UNTYPED_TYPE,
	namespaceNodeSeq = SEQUENCE_ARITY + NAMESPACE_NODE + SEQUENCE_REP + UNTYPED_TYPE,
	namespaceNodeSeqRef = REF_OBJECT + SEQUENCE_ARITY + NAMESPACE_NODE + REFERENCE_REP + UNTYPED_TYPE,
	namespaceNodePlus = NONEMPTY_SEQ_ARITY + NAMESPACE_NODE + NO_REP + UNTYPED_TYPE,
	namespaceNodeOpt = OPT_ITEM_ARITY + NAMESPACE_NODE + NO_REP + UNTYPED_TYPE,
	processingInstructionNode = ITEM_ARITY + PI_NODE + PI_REP + UNTYPED_TYPE,
	processingInstructionNodeRef = REF_OBJECT + ITEM_ARITY + PI_NODE + REFERENCE_REP + UNTYPED_TYPE,
	processingInstructionNodeSeq = SEQUENCE_ARITY + PI_NODE + SEQUENCE_REP + UNTYPED_TYPE,
	processingInstructionNodeSeqRef = REF_OBJECT + SEQUENCE_ARITY + PI_NODE + REFERENCE_REP + UNTYPED_TYPE,
	processingInstructionNodePlus = NONEMPTY_SEQ_ARITY + PI_NODE + NO_REP + UNTYPED_TYPE,
	processingInstructionNodeOpt = OPT_ITEM_ARITY + PI_NODE + NO_REP + UNTYPED_TYPE,
	commentNode = ITEM_ARITY + COMMENT_NODE + COMMENT_REP + UNTYPED_TYPE,
	commentNodeRef = REF_OBJECT + ITEM_ARITY + COMMENT_NODE + REFERENCE_REP + UNTYPED_TYPE,
	commentNodeSeq = SEQUENCE_ARITY + COMMENT_NODE + SEQUENCE_REP + UNTYPED_TYPE,
	commentNodeSeqRef = REF_OBJECT + SEQUENCE_ARITY + COMMENT_NODE + REFERENCE_REP + UNTYPED_TYPE,
	commentNodePlus = NONEMPTY_SEQ_ARITY + COMMENT_NODE + NO_REP + UNTYPED_TYPE,
	commentNodeOpt = OPT_ITEM_ARITY + COMMENT_NODE + NO_REP + UNTYPED_TYPE,
	textNode = ITEM_ARITY + TEXT_NODE + TEXT_REP + UNTYPED_TYPE,
	textNodeRef = REF_OBJECT + ITEM_ARITY + TEXT_NODE + REFERENCE_REP + UNTYPED_TYPE,
	textNodeSeq = SEQUENCE_ARITY + TEXT_NODE + SEQUENCE_REP + UNTYPED_TYPE,
	textNodeSeqRef = REF_OBJECT + SEQUENCE_ARITY + TEXT_NODE + REFERENCE_REP + UNTYPED_TYPE,
	textNodePlus = NONEMPTY_SEQ_ARITY + TEXT_NODE + NO_REP + UNTYPED_TYPE,
	textNodeOpt = OPT_ITEM_ARITY + TEXT_NODE + NO_REP + UNTYPED_TYPE,
	anyNode = ITEM_ARITY + ANY_NODE + NO_REP + UNTYPED_TYPE,
	anyNodeRef = REF_OBJECT + ITEM_ARITY + ANY_NODE + REFERENCE_REP + UNTYPED_TYPE,
	anyNodeSeq = SEQUENCE_ARITY + ANY_NODE + SEQUENCE_REP + UNTYPED_TYPE,
	anyNodeSeqRef = REF_OBJECT + SEQUENCE_ARITY + ANY_NODE + REFERENCE_REP + UNTYPED_TYPE,
	anyNodePlus = NONEMPTY_SEQ_ARITY + ANY_NODE + NO_REP + UNTYPED_TYPE,
	anyNodeOpt = OPT_ITEM_ARITY + ANY_NODE + NO_REP + UNTYPED_TYPE,

	// String values.
	xs_string = ITEM_ARITY + NOT_NODE + STRING_REP + STRING_TYPE,
	xs_stringRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + STRING_TYPE,
	xs_stringSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + STRING_TYPE,
	xs_stringSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + STRING_TYPE,
	xs_stringPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + STRING_TYPE,
	xs_stringOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + STRING_TYPE,
	xs_normalizedString = ITEM_ARITY + NOT_NODE + STRING_REP + NORMALIZED_STRING_SUB,
	xs_normalizedStringRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + NORMALIZED_STRING_SUB,
	xs_normalizedStringSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + NORMALIZED_STRING_SUB,
	xs_normalizedStringSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + NORMALIZED_STRING_SUB,
	xs_normalizedStringPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + NORMALIZED_STRING_SUB,
	xs_normalizedStringOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + NORMALIZED_STRING_SUB,
	xs_token = ITEM_ARITY + NOT_NODE + STRING_REP + TOKEN_SUB2,
	xs_tokenRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + TOKEN_SUB2,
	xs_tokenSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + TOKEN_SUB2,
	xs_tokenSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + TOKEN_SUB2,
	xs_tokenPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + TOKEN_SUB2,
	xs_tokenOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + TOKEN_SUB2,
	xs_language = ITEM_ARITY + NOT_NODE + STRING_REP + LANGUAGE_SUB3,
	xs_languageRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + LANGUAGE_SUB3,
	xs_languageSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + LANGUAGE_SUB3,
	xs_languageSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + LANGUAGE_SUB3,
	xs_languagePlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + LANGUAGE_SUB3,
	xs_languageOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + LANGUAGE_SUB3,
	xs_nmtoken = ITEM_ARITY + NOT_NODE + STRING_REP + NMTOKEN_SUB3,
	xs_nmtokenRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + NMTOKEN_SUB3,
	xs_nmtokenSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + NMTOKEN_SUB3,
	xs_nmtokenSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + NMTOKEN_SUB3,
	xs_nmtokenPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + NMTOKEN_SUB3,
	xs_nmtokenOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + NMTOKEN_SUB3,
	xs_name = ITEM_ARITY + NOT_NODE + STRING_REP + NAME_SUB3,
	xs_nameRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + NAME_SUB3,
	xs_nameSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + NAME_SUB3,
	xs_nameSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + NAME_SUB3,
	xs_namePlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + NAME_SUB3,
	xs_nameOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + NAME_SUB3,
	xs_ncName = ITEM_ARITY + NOT_NODE + STRING_REP + NCNAME_SUB4,
	xs_ncNameRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + NCNAME_SUB4,
	xs_ncNameSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + NCNAME_SUB4,
	xs_ncNameSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + NCNAME_SUB4,
	xs_ncNamePlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + NCNAME_SUB4,
	xs_ncNameOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + NCNAME_SUB4,
	xs_id = ITEM_ARITY + NOT_NODE + STRING_REP + ID_SUB5,
	xs_idRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + ID_SUB5,
	xs_idSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + ID_SUB5,
	xs_idSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + ID_SUB5,
	xs_idPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + ID_SUB5,
	xs_idOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + ID_SUB5,
	xs_idref = ITEM_ARITY + NOT_NODE + STRING_REP + IDREF_SUB5,
	xs_idrefRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + IDREF_SUB5,
	xs_idrefSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + IDREF_SUB5,
	xs_idrefSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + IDREF_SUB5,
	xs_idrefPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + IDREF_SUB5,
	xs_idrefOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + IDREF_SUB5,
	xs_entity = ITEM_ARITY + NOT_NODE + STRING_REP + ENTITY_SUB5,
	xs_entityRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + ENTITY_SUB5,
	xs_entitySeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + ENTITY_SUB5,
	xs_entitySeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + ENTITY_SUB5,
	xs_entityPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + ENTITY_SUB5,
	xs_entityOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + ENTITY_SUB5,

	// Numeric values.
	//xs_decimal = ITEM_ARITY + NOT_NODE + DECIMAL_REP + DECIMAL_TYPE, 
	//xs_decimalRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + DECIMAL_TYPE, 
	//xs_decimalSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + DECIMAL_TYPE, 
	//xs_decimalSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + DECIMAL_TYPE, 
	//xs_decimalPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + DECIMAL_TYPE, 
	//xs_decimalOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + DECIMAL_TYPE, 
	xs_decimal = ITEM_ARITY + NOT_NODE + DOUBLE_REP + DECIMAL_TYPE,  // no decimal rep yet
	xs_decimalRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + DECIMAL_TYPE,  // no decimal rep yet
	xs_decimalSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + DECIMAL_TYPE,  // no decimal rep yet
	xs_decimalSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + DECIMAL_TYPE,  // no decimal rep yet
	xs_decimalPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + DECIMAL_TYPE,  // no decimal rep yet
	xs_decimalOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + DECIMAL_TYPE,  // no decimal rep yet
	//xs_integer = ITEM_ARITY + NOT_NODE + DECIMAL_REP + INTEGER_SUB,
	//xs_integerRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + INTEGER_SUB,
	//xs_integerSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + INTEGER_SUB,
	//xs_integerSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + INTEGER_SUB,
	//xs_integerPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + INTEGER_SUB,
	//xs_integerOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + INTEGER_SUB,
	xs_integer = ITEM_ARITY + NOT_NODE + DOUBLE_REP + INTEGER_SUB,	// no decimal rep yet
	xs_integerRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + INTEGER_SUB,	// no decimal rep yet
	xs_integerSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + INTEGER_SUB,	// no decimal rep yet
	xs_integerSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + INTEGER_SUB,	// no decimal rep yet
	xs_integerPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + INTEGER_SUB,	// no decimal rep yet
	xs_integerOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + INTEGER_SUB,	// no decimal rep yet
	//xs_nonPositiveInteger = ITEM_ARITY + NOT_NODE + DECIMAL_REP + NON_POSITIVE_INTEGER_SUB2, 
	//xs_nonPositiveIntegerRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + NON_POSITIVE_INTEGER_SUB2, 
	//xs_nonPositiveIntegerSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + NON_POSITIVE_INTEGER_SUB2, 
	//xs_nonPositiveIntegerSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + NON_POSITIVE_INTEGER_SUB2, 
	//xs_nonPositiveIntegerPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + NON_POSITIVE_INTEGER_SUB2, 
	//xs_nonPositiveIntegerOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + NON_POSITIVE_INTEGER_SUB2, 
	xs_nonPositiveInteger = ITEM_ARITY + NOT_NODE + DOUBLE_REP + NON_POSITIVE_INTEGER_SUB2,	// no decimal rep yet
	xs_nonPositiveIntegerRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + NON_POSITIVE_INTEGER_SUB2,	// no decimal rep yet
	xs_nonPositiveIntegerSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + NON_POSITIVE_INTEGER_SUB2,	// no decimal rep yet
	xs_nonPositiveIntegerSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + NON_POSITIVE_INTEGER_SUB2,	// no decimal rep yet
	xs_nonPositiveIntegerPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + NON_POSITIVE_INTEGER_SUB2,	// no decimal rep yet
	xs_nonPositiveIntegerOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + NON_POSITIVE_INTEGER_SUB2,	// no decimal rep yet
	//xs_negativeInteger = ITEM_ARITY + NOT_NODE + DECIMAL_REP + NEGATIVE_INTEGER_SUB3,
	//xs_negativeIntegerRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + NEGATIVE_INTEGER_SUB3,
	//xs_negativeIntegerSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + NEGATIVE_INTEGER_SUB3,
	//xs_negativeIntegerSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + NEGATIVE_INTEGER_SUB3,
	//xs_negativeIntegerPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + NEGATIVE_INTEGER_SUB3,
	//xs_negativeIntegerOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + NEGATIVE_INTEGER_SUB3,
	xs_negativeInteger = ITEM_ARITY + NOT_NODE + DOUBLE_REP + NEGATIVE_INTEGER_SUB3,	// no decimal rep yet
	xs_negativeIntegerRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + NEGATIVE_INTEGER_SUB3,	// no decimal rep yet
	xs_negativeIntegerSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + NEGATIVE_INTEGER_SUB3,	// no decimal rep yet
	xs_negativeIntegerSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + NEGATIVE_INTEGER_SUB3,	// no decimal rep yet
	xs_negativeIntegerPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + NEGATIVE_INTEGER_SUB3,	// no decimal rep yet
	xs_negativeIntegerOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + NEGATIVE_INTEGER_SUB3,	// no decimal rep yet
	//xs_long = ITEM_ARITY + NOT_NODE + DECIMAL_REP + LONG_SUB2,
	//xs_longRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + LONG_SUB2,
	//xs_longSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + LONG_SUB2,
	//xs_longSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + LONG_SUB2,
	//xs_longPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + LONG_SUB2,
	//xs_longOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + LONG_SUB2,
	xs_long = ITEM_ARITY + NOT_NODE + DOUBLE_REP + LONG_SUB2,	// no decimal rep yet
	xs_longRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + LONG_SUB2,	// no decimal rep yet
	xs_longSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + LONG_SUB2,	// no decimal rep yet
	xs_longSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + LONG_SUB2,	// no decimal rep yet
	xs_longPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + LONG_SUB2,	// no decimal rep yet
	xs_longOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + LONG_SUB2,	// no decimal rep yet
	xs_int = ITEM_ARITY + NOT_NODE + DOUBLE_REP + INT_SUB3,
	xs_intRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + INT_SUB3,
	xs_intSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + INT_SUB3,
	xs_intSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + INT_SUB3,
	xs_intPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + INT_SUB3,
	xs_intOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + INT_SUB3,
	xs_short = ITEM_ARITY + NOT_NODE + DOUBLE_REP + SHORT_SUB4,
	xs_shortRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + SHORT_SUB4,
	xs_shortSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + SHORT_SUB4,
	xs_shortSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + SHORT_SUB4,
	xs_shortPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + SHORT_SUB4,
	xs_shortOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + SHORT_SUB4,
	xs_byte = ITEM_ARITY + NOT_NODE + DECIMAL_REP + BYTE_SUB5,
	xs_byteRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + BYTE_SUB5,
	xs_byteSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + BYTE_SUB5,
	xs_byteSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + BYTE_SUB5,
	xs_bytePlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + BYTE_SUB5,
	xs_byteOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + BYTE_SUB5,
	//xs_nonNegativeInteger = ITEM_ARITY + NOT_NODE + DECIMAL_REP + NON_EGATIVE_INTEGER_SUB2,
	//xs_nonNegativeIntegerRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + NON_EGATIVE_INTEGER_SUB2,
	//xs_nonNegativeIntegerSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + NON_EGATIVE_INTEGER_SUB2,
	//xs_nonNegativeIntegerSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + NON_EGATIVE_INTEGER_SUB2,
	//xs_nonNegativeIntegerPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + NON_EGATIVE_INTEGER_SUB2,
	//xs_nonNegativeIntegerOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + NON_EGATIVE_INTEGER_SUB2,
	xs_nonNegativeInteger = ITEM_ARITY + NOT_NODE + DOUBLE_REP + NON_NEGATIVE_INTEGER_SUB2,  // no decimal rep yet
	xs_nonNegativeIntegerRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + NON_NEGATIVE_INTEGER_SUB2,  // no decimal rep yet
	xs_nonNegativeIntegerSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + NON_NEGATIVE_INTEGER_SUB2,  // no decimal rep yet
	xs_nonNegativeIntegerSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + NON_NEGATIVE_INTEGER_SUB2,  // no decimal rep yet
	xs_nonNegativeIntegerPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + NON_NEGATIVE_INTEGER_SUB2,  // no decimal rep yet
	xs_nonNegativeIntegerOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + NON_NEGATIVE_INTEGER_SUB2,  // no decimal rep yet
	//xs_unsignedLong = ITEM_ARITY + NOT_NODE + DECIMAL_REP + UNSIGNED_LONG_SUB3,
	//xs_unsignedLongRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + UNSIGNED_LONG_SUB3,
	//xs_unsignedLongSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + UNSIGNED_LONG_SUB3,
	//xs_unsignedLongSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + UNSIGNED_LONG_SUB3,
	//xs_unsignedLongPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + UNSIGNED_LONG_SUB3,
	//xs_unsignedLongOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + UNSIGNED_LONG_SUB3,
	xs_unsignedLong = ITEM_ARITY + NOT_NODE + DOUBLE_REP + UNSIGNED_LONG_SUB3,	// no decimal rep yet
	xs_unsignedLongRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + UNSIGNED_LONG_SUB3,	// no decimal rep yet
	xs_unsignedLongSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + UNSIGNED_LONG_SUB3,	// no decimal rep yet
	xs_unsignedLongSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + UNSIGNED_LONG_SUB3,	// no decimal rep yet
	xs_unsignedLongPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + UNSIGNED_LONG_SUB3,	// no decimal rep yet
	xs_unsignedLongOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + UNSIGNED_LONG_SUB3,	// no decimal rep yet
	xs_unsignedInt = ITEM_ARITY + NOT_NODE + DECIMAL_REP + UNSIGNED_INT_SUB4,
	xs_unsignedIntRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + UNSIGNED_INT_SUB4,
	xs_unsignedIntSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + UNSIGNED_INT_SUB4,
	xs_unsignedIntSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + UNSIGNED_INT_SUB4,
	xs_unsignedIntPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + UNSIGNED_INT_SUB4,
	xs_unsignedIntOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + UNSIGNED_INT_SUB4,
	xs_unsignedShort = ITEM_ARITY + NOT_NODE + DOUBLE_REP + UNSIGNED_SHORT_SUB5,
	xs_unsignedShortRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + UNSIGNED_SHORT_SUB5,
	xs_unsignedShortSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + UNSIGNED_SHORT_SUB5,
	xs_unsignedShortSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + UNSIGNED_SHORT_SUB5,
	xs_unsignedShortPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + UNSIGNED_SHORT_SUB5,
	xs_unsignedShortOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + UNSIGNED_SHORT_SUB5,
	xs_unsignedByte = ITEM_ARITY + NOT_NODE + DECIMAL_REP + UNSIGNED_BYTE_SUB6,
	xs_unsignedByteRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + UNSIGNED_BYTE_SUB6,
	xs_unsignedByteSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + UNSIGNED_BYTE_SUB6,
	xs_unsignedByteSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + UNSIGNED_BYTE_SUB6,
	xs_unsignedBytePlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + UNSIGNED_BYTE_SUB6,
	xs_unsignedByteOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + UNSIGNED_BYTE_SUB6,
//xs_positiveInteger = ITEM_ARITY + NOT_NODE + DECIMAL_REP + POSITIVE_INTEGER_SUB3,  // no decimal rep yet
//xs_positiveIntegerRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + POSITIVE_INTEGER_SUB3,  // no decimal rep yet
//xs_positiveIntegerSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + POSITIVE_INTEGER_SUB3,  // no decimal rep yet
//xs_positiveIntegerSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + POSITIVE_INTEGER_SUB3,  // no decimal rep yet
//xs_positiveIntegerPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + POSITIVE_INTEGER_SUB3,  // no decimal rep yet
//xs_positiveIntegerOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + POSITIVE_INTEGER_SUB3,  // no decimal rep yet
	xs_positiveInteger = ITEM_ARITY + NOT_NODE + DOUBLE_REP + POSITIVE_INTEGER_SUB3,
	xs_positiveIntegerRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + POSITIVE_INTEGER_SUB3,
	xs_positiveIntegerSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + POSITIVE_INTEGER_SUB3,
	xs_positiveIntegerSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + POSITIVE_INTEGER_SUB3,
	xs_positiveIntegerPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + POSITIVE_INTEGER_SUB3,
	xs_positiveIntegerOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + POSITIVE_INTEGER_SUB3,

	// Other atomic values.
	xs_untypedValue = ITEM_ARITY + NOT_NODE + STRING_REP + UNTYPED_TYPE,
	xs_untypedValueRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + UNTYPED_TYPE,
	xs_untypedValueSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + UNTYPED_TYPE,
	xs_untypedValueSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + UNTYPED_TYPE,
	xs_untypedValuePlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + UNTYPED_TYPE,
	xs_untypedValueOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + UNTYPED_TYPE,
	xs_untypedAtomicValue = ITEM_ARITY + NOT_NODE + STRING_REP + UNTYPED_ATOMIC_TYPE,
	xs_untypedAtomicValueRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + UNTYPED_ATOMIC_TYPE,
	xs_untypedAtomicValueSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + UNTYPED_ATOMIC_TYPE,
	xs_untypedAtomicValueSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + UNTYPED_ATOMIC_TYPE,
	xs_untypedAtomicValuePlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + UNTYPED_ATOMIC_TYPE,
	xs_untypedAtomicValueOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + UNTYPED_ATOMIC_TYPE,
	xs_boolean = ITEM_ARITY + NOT_NODE + BOOLEAN_REP + BOOLEAN_TYPE,
	xs_booleanRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + BOOLEAN_TYPE,
	xs_booleanSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + BOOLEAN_TYPE,
	xs_booleanSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + BOOLEAN_TYPE,
	xs_booleanPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + BOOLEAN_TYPE,
	xs_booleanOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + BOOLEAN_TYPE,
	xs_float = ITEM_ARITY + NOT_NODE + DOUBLE_REP + FLOAT_TYPE,
	xs_floatRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + FLOAT_TYPE,
	xs_floatSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + FLOAT_TYPE,
	xs_floatSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + FLOAT_TYPE,
	xs_floatPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + FLOAT_TYPE,
	xs_floatOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + FLOAT_TYPE,
	xs_double = ITEM_ARITY + NOT_NODE + DOUBLE_REP + DOUBLE_TYPE,
	xs_doubleRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + DOUBLE_TYPE,
	xs_doubleSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + DOUBLE_TYPE,
	xs_doubleSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + DOUBLE_TYPE,
	xs_doublePlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + DOUBLE_TYPE,
	xs_doubleOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + DOUBLE_TYPE,
	xs_duration = ITEM_ARITY + NOT_NODE + TIME_REP + DURATION_TYPE,
	xs_durationRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + DURATION_TYPE,
	xs_durationSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + DURATION_TYPE,
	xs_durationSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + DURATION_TYPE,
	xs_durationPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + DURATION_TYPE,
	xs_durationOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + DURATION_TYPE,
	xs_dayTimeDuration = ITEM_ARITY + NOT_NODE + TIME_REP + DAY_TIME_DURATION_SUB,
	xs_dayTimeDurationRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + DAY_TIME_DURATION_SUB,
	xs_dayTimeDurationSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + DAY_TIME_DURATION_SUB,
	xs_dayTimeDurationSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + DAY_TIME_DURATION_SUB,
	xs_dayTimeDurationPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + DAY_TIME_DURATION_SUB,
	xs_dayTimeDurationOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + DAY_TIME_DURATION_SUB,
	xs_yearMonthDuration = ITEM_ARITY + NOT_NODE + TIME_REP + YEAR_MONTH_DURATION_SUB,
	xs_yearMonthDurationRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + YEAR_MONTH_DURATION_SUB,
	xs_yearMonthDurationSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + YEAR_MONTH_DURATION_SUB,
	xs_yearMonthDurationSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + YEAR_MONTH_DURATION_SUB,
	xs_yearMonthDurationPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + YEAR_MONTH_DURATION_SUB,
	xs_yearMonthDurationOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + YEAR_MONTH_DURATION_SUB,
	xs_dateTime = ITEM_ARITY + NOT_NODE + TIME_REP + DATE_TIME_TYPE,
	xs_dateTimeRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + DATE_TIME_TYPE,
	xs_dateTimeSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + DATE_TIME_TYPE,
	xs_dateTimeSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + DATE_TIME_TYPE,
	xs_dateTimePlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + DATE_TIME_TYPE,
	xs_dateTimeOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + DATE_TIME_TYPE,
	xs_time = ITEM_ARITY + NOT_NODE + TIME_REP + TIME_TYPE,
	xs_timeRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + TIME_TYPE,
	xs_timeSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + TIME_TYPE,
	xs_timeSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + TIME_TYPE,
	xs_timePlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + TIME_TYPE,
	xs_timeOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + TIME_TYPE,
	xs_date = ITEM_ARITY + NOT_NODE + TIME_REP + DATE_TYPE,
	xs_dateRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + DATE_TYPE,
	xs_dateSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + DATE_TYPE,
	xs_dateSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + DATE_TYPE,
	xs_datePlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + DATE_TYPE,
	xs_dateOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + DATE_TYPE,
	xs_gYearMonth = ITEM_ARITY + NOT_NODE + TIME_REP + G_YEAR_MONTH_TYPE,
	xs_gYearMonthRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + G_YEAR_MONTH_TYPE,
	xs_gYearMonthSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + G_YEAR_MONTH_TYPE,
	xs_gYearMonthSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + G_YEAR_MONTH_TYPE,
	xs_gYearMonthPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + G_YEAR_MONTH_TYPE,
	xs_gYearMonthOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + G_YEAR_MONTH_TYPE,
	xs_gYear = ITEM_ARITY + NOT_NODE + TIME_REP + G_YEAR_TYPE,
	xs_gYearRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + G_YEAR_TYPE,
	xs_gYearSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + G_YEAR_TYPE,
	xs_gYearSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + G_YEAR_TYPE,
	xs_gYearPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + G_YEAR_TYPE,
	xs_gYearOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + G_YEAR_TYPE,
	xs_gMonthDay = ITEM_ARITY + NOT_NODE + TIME_REP + G_MONTH_DAY_TYPE,
	xs_gMonthDayRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + G_MONTH_DAY_TYPE,
	xs_gMonthDaySeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + G_MONTH_DAY_TYPE,
	xs_gMonthDaySeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + G_MONTH_DAY_TYPE,
	xs_gMonthDayPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + G_MONTH_DAY_TYPE,
	xs_gMonthDayOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + G_MONTH_DAY_TYPE,
	xs_gDay = ITEM_ARITY + NOT_NODE + TIME_REP + G_DAY_TYPE,
	xs_gDayRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + G_DAY_TYPE,
	xs_gDaySeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + G_DAY_TYPE,
	xs_gDaySeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + G_DAY_TYPE,
	xs_gDayPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + G_DAY_TYPE,
	xs_gDayOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + G_DAY_TYPE,
	xs_gMonth = ITEM_ARITY + NOT_NODE + TIME_REP + G_MONTH_TYPE,
	xs_gMonthRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + G_MONTH_TYPE,
	xs_gMonthSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + G_MONTH_TYPE,
	xs_gMonthSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + G_MONTH_TYPE,
	xs_gMonthPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + G_MONTH_TYPE,
	xs_gMonthOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + G_MONTH_TYPE,
	xs_hexBinary = ITEM_ARITY + NOT_NODE + BINARY_REP + HEX_BINARY_TYPE,
	xs_hexBinaryRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + HEX_BINARY_TYPE,
	xs_hexBinarySeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + HEX_BINARY_TYPE,
	xs_hexBinarySeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + HEX_BINARY_TYPE,
	xs_hexBinaryPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + HEX_BINARY_TYPE,
	xs_hexBinaryOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + HEX_BINARY_TYPE,
	xs_base64Binary = ITEM_ARITY + NOT_NODE + BINARY_REP + BASE_64_BINARY_TYPE,
	xs_base64BinaryRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + BASE_64_BINARY_TYPE,
	xs_base64BinarySeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + BASE_64_BINARY_TYPE,
	xs_base64BinarySeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + BASE_64_BINARY_TYPE,
	xs_base64BinaryPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + BASE_64_BINARY_TYPE,
	xs_base64BinaryOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + BASE_64_BINARY_TYPE,
	xs_anyURI = ITEM_ARITY + NOT_NODE + STRING_REP + ANY_URI_TYPE,
	xs_anyURIRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + ANY_URI_TYPE,
	xs_anyURISeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + ANY_URI_TYPE,
	xs_anyURISeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + ANY_URI_TYPE,
	xs_anyURIPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + ANY_URI_TYPE,
	xs_anyURIOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + ANY_URI_TYPE,
	xs_qname = ITEM_ARITY + NOT_NODE + QNAME_REP + QNAME_TYPE,
	xs_qnameRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + QNAME_TYPE,
	xs_qnameSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + QNAME_TYPE,
	xs_qnameSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + QNAME_TYPE,
	xs_qnamePlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + QNAME_TYPE,
	xs_qnameOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + QNAME_TYPE,
	xs_notation = ITEM_ARITY + NOT_NODE + QNAME_REP + NOTATION_TYPE,
	xs_notationRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + NOTATION_TYPE,
	xs_notationSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + NOTATION_TYPE,
	xs_notationSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + NOTATION_TYPE,
	xs_notationPlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + NOTATION_TYPE,
	xs_notationOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + NOTATION_TYPE,
	xs_complexType = ITEM_ARITY + NOT_NODE + QNAME_REP + NOTATION_TYPE,
	xs_complexTypeRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + NOTATION_TYPE,
	xs_complexTypeSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + NOTATION_TYPE,
	xs_complexTypeSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + NOTATION_TYPE,
	xs_complexTypePlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + NOTATION_TYPE,
	xs_complexTypeOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + NOTATION_TYPE,
	xs_emptyType = ITEM_ARITY + NOT_NODE + NO_REP + EMPTY_TYPE,
	xs_emptyTypeRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + EMPTY_TYPE,
	xs_emptyTypeSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + EMPTY_TYPE,
	xs_emptyTypeSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + EMPTY_TYPE,
	xs_emptyTypePlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + EMPTY_TYPE,
	xs_emptyTypeOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + EMPTY_TYPE,
	xs_anyType = ITEM_ARITY + NOT_NODE + NO_REP + ANY_TYPE,
	xs_anyTypeRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + ANY_TYPE,
	xs_anyTypeSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + ANY_TYPE,
	xs_anyTypeSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + ANY_TYPE,
	xs_anyTypePlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + ANY_TYPE,
	xs_anyTypeOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + ANY_TYPE,
	xs_rawBinaryType = ITEM_ARITY + NOT_NODE + BINARY_REP + RAW_BINARY_TYPE,
	xs_rawBinaryTypeRef = REF_OBJECT + ITEM_ARITY + NOT_NODE + REFERENCE_REP + RAW_BINARY_TYPE,
	xs_rawBinaryTypeSeq = SEQUENCE_ARITY + NOT_NODE + SEQUENCE_REP + RAW_BINARY_TYPE,
	xs_rawBinaryTypeSeqRef = REF_OBJECT + SEQUENCE_ARITY + NOT_NODE + REFERENCE_REP + RAW_BINARY_TYPE,
	xs_rawBinaryTypePlus = NONEMPTY_SEQ_ARITY + NOT_NODE + NO_REP + RAW_BINARY_TYPE,
	xs_rawBinaryTypeOpt = OPT_ITEM_ARITY + NOT_NODE + NO_REP + RAW_BINARY_TYPE,
};

} /* namespace xqp */
#endif /* XQP_TYPECODES_H */

