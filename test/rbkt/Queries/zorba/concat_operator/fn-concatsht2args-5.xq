(:*******************************************************:)
(:Test: concatsht2args-5                                  :)
(:Written By: Carmelo Montanez                            :)
(:Date: Wed Dec 15 15:41:48 GMT-05:00 2004                :)
(:Purpose: Evaluates The "concat" function               :)
(: with the arguments set as follows:                    :)
(:$arg1 = xs:short(lower bound)                          :)
(:$arg2 = xs:short(upper bound)                          :)
(:*******************************************************:)

xs:short("-32768") || xs:short("32767")