#ifndef XQP_PRINTERVISITOR_H
#define XQP_PRINTERVISITOR_H

#include "runtime/visitors/planitervisitor.h"
#include "runtime/visitors/iterprinter.h"

namespace xqp {
  class PlanIterator;
  /**
   * Visitor implementation to print a tree.
   */
  class PrinterVisitor : public PlanIterVisitor {
    private:
      IterPrinter& thePrinter;
      
    public:
      PrinterVisitor(IterPrinter& aPrinter) : thePrinter(aPrinter) {}
      
    private:
      void printCommons(const PlanIterator* aIter);
      
    public:
      virtual ~PrinterVisitor() {}
      
      virtual void beginVisit ( const FnDataIterator& ) ;
      virtual void endVisit ( const FnDataIterator& ) ;
       
      virtual void beginVisit ( const FnRootIterator& ) ;
      virtual void endVisit ( const FnRootIterator& ) ;
       
      virtual void beginVisit ( const FnBooleanIterator& ) ;
      virtual void endVisit ( const FnBooleanIterator& ) ;
       
      virtual void beginVisit ( const LogicIterator& ) ;
      virtual void endVisit ( const LogicIterator& ) ;
       
      virtual void beginVisit ( const CompareIterator& ) ;
      virtual void endVisit ( const CompareIterator& ) ;
       
      virtual void beginVisit ( const ElementIterator& ) ;
      virtual void endVisit ( const ElementIterator& ) ;
       
      virtual void beginVisit ( const ElementContentIterator& ) ;
      virtual void endVisit ( const ElementContentIterator& ) ;
       
      virtual void beginVisit ( const AttributeIterator & ) ;
      virtual void endVisit ( const AttributeIterator& ) ;
       
      virtual void beginVisit ( const CommentIterator& ) ;
      virtual void endVisit ( const CommentIterator& ) ;
       
      virtual void beginVisit ( const RefIterator& ) ;
      virtual void endVisit ( const RefIterator& ) ;
       
      virtual void beginVisit ( const SingletonIterator& ) ;
      virtual void endVisit ( const SingletonIterator& ) ;
       
      virtual void beginVisit ( const EmptyIterator& ) ;
      virtual void endVisit ( const EmptyIterator& ) ;
       
      virtual void beginVisit ( const var_iterator& ) ;
      virtual void endVisit ( const var_iterator& ) ;
       
      virtual void beginVisit ( const EnclosedIterator& ) ;
      virtual void endVisit ( const EnclosedIterator& ) ;
       
      virtual void beginVisit ( const IfThenElseIterator& ) ;
      virtual void endVisit ( const IfThenElseIterator& ) ;
       
      virtual void beginVisit ( const FLWORIterator& ) ;
      virtual void endVisit ( const FLWORIterator& ) ;
      
      virtual void beginVisit ( const NodeDistinctIterator& a );
      virtual void endVisit ( const NodeDistinctIterator& a );

      virtual void beginVisit ( const NodeSortIterator& a );
      virtual void endVisit ( const NodeSortIterator& a );

      virtual void beginVisit ( const KindTestIterator& ) ;
      virtual void endVisit ( const KindTestIterator& ) ;
       
      virtual void beginVisit ( const NameTestIterator& ) ;
      virtual void endVisit ( const NameTestIterator& ) ;
       
      virtual void beginVisit ( const SelfAxisIterator& ) ;
      virtual void endVisit ( const SelfAxisIterator& ) ;
       
      virtual void beginVisit ( const AttributeAxisIterator& ) ;
      virtual void endVisit ( const AttributeAxisIterator& ) ;
       
      virtual void beginVisit ( const ParentAxisIterator& ) ;
      virtual void endVisit ( const ParentAxisIterator& ) ;
       
      virtual void beginVisit ( const AncestorAxisIterator& ) ;
      virtual void endVisit ( const AncestorAxisIterator& ) ;
       
      virtual void beginVisit ( const AncestorSelfAxisIterator& ) ;
      virtual void endVisit ( const AncestorSelfAxisIterator& ) ;
       
      virtual void beginVisit ( const RSiblingAxisIterator& ) ;
      virtual void endVisit ( const RSiblingAxisIterator& ) ;
       
      virtual void beginVisit ( const LSiblingAxisIterator& ) ;
      virtual void endVisit ( const LSiblingAxisIterator& ) ;
       
      virtual void beginVisit ( const ChildAxisIterator& ) ;
      virtual void endVisit ( const ChildAxisIterator& ) ;
       
      virtual void beginVisit ( const DescendantAxisIterator& ) ;
      virtual void endVisit ( const DescendantAxisIterator& ) ;
       
      virtual void beginVisit ( const DescendantSelfAxisIterator& ) ;
      virtual void endVisit ( const DescendantSelfAxisIterator& ) ;
       
      virtual void beginVisit ( const PrecedingAxisIterator& ) ;
      virtual void endVisit ( const PrecedingAxisIterator& ) ;
       
      virtual void beginVisit ( const FollowingAxisIterator& ) ;
      virtual void endVisit ( const FollowingAxisIterator& ) ;
       
      virtual void beginVisit ( const InstanceOfIterator& ) ;
      virtual void endVisit ( const InstanceOfIterator& ) ;       
       
      virtual void beginVisit ( const ArithmeticIterator<AddOperations>& ) ;
      virtual void beginVisit ( const ArithmeticIterator<SubtractOperations>& ) ;
      virtual void beginVisit ( const ArithmeticIterator<MultiplyOperations>& ) ;
      virtual void beginVisit ( const ArithmeticIterator<DivideOperations>& ) ;
      virtual void beginVisit ( const ArithmeticIterator<IntegerDivideOperations>& ) ;
      virtual void beginVisit ( const ArithmeticIterator<ModOperations>& ) ;
      virtual void endVisit ( const ArithmeticIterator<AddOperations>& ) ;
      virtual void endVisit ( const ArithmeticIterator<SubtractOperations>& ) ;
      virtual void endVisit ( const ArithmeticIterator<MultiplyOperations>& ) ;
      virtual void endVisit ( const ArithmeticIterator<DivideOperations>& ) ;
      virtual void endVisit ( const ArithmeticIterator<IntegerDivideOperations>& ) ;
      virtual void endVisit ( const ArithmeticIterator<ModOperations>& ) ;
       
      virtual void beginVisit ( const OpNumericUnaryIterator& ) ;
      virtual void endVisit ( const OpNumericUnaryIterator& ) ;
       
      virtual void beginVisit ( const FnAbsIterator& ) ;
      virtual void endVisit ( const FnAbsIterator& ) ;

      virtual void beginVisit( const FnCeilingIterator& );
      virtual void endVisit( const FnCeilingIterator& );

      virtual void beginVisit( const FnFloorIterator& );
      virtual void endVisit( const FnFloorIterator& );

      virtual void beginVisit( const FnRoundIterator& );
      virtual void endVisit( const FnRoundIterator& );

      virtual void beginVisit( const FnRoundHalfToEvenIterator& );
      virtual void endVisit( const FnRoundHalfToEvenIterator& );

      virtual void beginVisit ( const FnConcatIterator& ) ;
      virtual void endVisit ( const FnConcatIterator& ) ;

      virtual void beginVisit ( const FnEmptyIterator& ) ;
      virtual void endVisit ( const FnEmptyIterator& ) ;

      virtual void beginVisit ( const FnExistsIterator& ) ;
      virtual void endVisit ( const FnExistsIterator& ) ;

      virtual void beginVisit ( const FnIndexOfIterator& ) ;
      virtual void endVisit ( const FnIndexOfIterator& ) ;

      virtual void beginVisit ( const FnRemoveIterator& ) ;
      virtual void endVisit ( const FnRemoveIterator& ) ;

      virtual void beginVisit ( const FnDistinctValuesIterator& ) ;
      virtual void endVisit   ( const FnDistinctValuesIterator& ) ;

      virtual void beginVisit ( const FnInsertBeforeIterator& ) ;
      virtual void endVisit   ( const FnInsertBeforeIterator& ) ;
              
      virtual void beginVisit ( const DocIterator& ) ;
      virtual void endVisit ( const DocIterator& ) ;
       
      virtual void beginVisit ( const CodepointsToStringIterator& ) ;
      virtual void endVisit ( const CodepointsToStringIterator& ) ;
       
      virtual void beginVisit ( const StringToCodepointsIterator& ) ;
      virtual void endVisit ( const StringToCodepointsIterator& ) ;
       
      virtual void beginVisit ( const CompareStrIterator& ) ;
      virtual void endVisit ( const CompareStrIterator& ) ;
       
      virtual void beginVisit ( const CodepointEqualIterator& ) ;
      virtual void endVisit ( const CodepointEqualIterator& ) ;
       
      virtual void beginVisit ( const ConcatStrIterator& ) ;
      virtual void endVisit ( const ConcatStrIterator& ) ;
       
      virtual void beginVisit ( const StringJoinIterator& ) ;
      virtual void endVisit ( const StringJoinIterator& ) ;
       
      virtual void beginVisit ( const SubstringIterator& ) ;
      virtual void endVisit ( const SubstringIterator& ) ;
       
      virtual void beginVisit ( const StringLengthIterator& ) ;
      virtual void endVisit ( const StringLengthIterator& ) ;
       
      virtual void beginVisit ( const NormalizeSpaceIterator& ) ;
      virtual void endVisit ( const NormalizeSpaceIterator& ) ;
       
      virtual void beginVisit ( const NormalizeUnicodeIterator& ) ;
      virtual void endVisit ( const NormalizeUnicodeIterator& ) ;
       
      virtual void beginVisit ( const UpperCaseIterator& ) ;
      virtual void endVisit ( const UpperCaseIterator& ) ;
       
      virtual void beginVisit ( const LowerCaseIterator& ) ;
      virtual void endVisit ( const LowerCaseIterator& ) ;
       
      virtual void beginVisit ( const TranslateIterator& ) ;
      virtual void endVisit ( const TranslateIterator& ) ;
       
      virtual void beginVisit ( const EncodeForUriIterator& ) ;
      virtual void endVisit ( const EncodeForUriIterator& ) ;
       
      virtual void beginVisit ( const IriToUriIterator& ) ;
      virtual void endVisit ( const IriToUriIterator& ) ;
       
      virtual void beginVisit ( const EscapeHtmlUriIterator& ) ;
      virtual void endVisit ( const EscapeHtmlUriIterator& ) ;
       
      virtual void beginVisit ( const ContainsIterator& ) ;
      virtual void endVisit ( const ContainsIterator& ) ;
       
      virtual void beginVisit ( const StartsWithIterator& ) ;
      virtual void endVisit ( const StartsWithIterator& ) ;
       
      virtual void beginVisit ( const EndsWithIterator& ) ;
      virtual void endVisit ( const EndsWithIterator& ) ;
       
      virtual void beginVisit ( const SubstringBeforeIterator& ) ;
      virtual void endVisit ( const SubstringBeforeIterator& ) ;
       
      virtual void beginVisit ( const SubstringAfterIterator& ) ;
      virtual void endVisit ( const SubstringAfterIterator& ) ;
      
      virtual void beginVisit ( const ZorNumGen& ) ;
      virtual void endVisit ( const ZorNumGen& ) ;
      
      virtual void beginVisit ( const TextIterator& ) ;
      virtual void endVisit ( const TextIterator& ) ;
  };
} /* namespace xqp */

#endif
