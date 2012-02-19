(:*******************************************************:)
(:Test: concatdec2args-5                                  :)
(:Written By: Carmelo Montanez                            :)
(:Date: Wed Dec 15 15:41:48 GMT-05:00 2004                :)
(:Purpose: Evaluates The "concat" function               :)
(: with the arguments set as follows:                    :)
(:$arg1 = xs:decimal(lower bound)                        :)
(:$arg2 = xs:decimal(upper bound)                        :)
(:*******************************************************:)

xs:decimal("-999999999999999999") || xs:decimal("999999999999999999")
