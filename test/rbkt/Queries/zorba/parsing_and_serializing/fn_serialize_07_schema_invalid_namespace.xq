(: testing for error XQDY0027 if the wrong namespace name was provided :)

let $params :=
  <serialization-parameters xmlns="wrong_namespace">
  </serialization-parameters>
return
  fn:serialize(<a/>, $params)
