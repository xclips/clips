(printout "Hello" crlf)
(clear)
(deffunction foo (?a))
(foo (+ (eval "(gensym)") 2))
(clear)
(defmethod foo (?a))
(foo (instances))
(clear)
(deffunction foo ()
  (+ ?a 2)
  (bind ?a 1))
(foo)
(clear)
(deffunction foo ()
  (printout t "Hi there!" crlf))
(deffunction bar ()
  (foo)
  (undeffunction *))
(bar)
(clear)
(defmethod foo ()
  (undefmethod foo *))
(foo)
(clear)
(defmethod foo 1 ())
(ppdefmethod foo 2)
(clear)
(defgeneric foo)
(undefmethod foo junk)
(clear)
(defmethod foo ()
  (+ ?a 2)
  (bind ?a 1))
(foo)
(clear)
(defmethod foo ())
(foo 1 2)
(clear)
(call-next-method)
(clear)
(defmethod foo ((?a INTEGER)))
(foo [bogus-instance])
(clear)
(undefmethod * 1)
(clear)
(instance-address [bogus-instance])
(clear)
(defclass A (is-a USER) (role concrete))
(make-instance a of A)
(defglobal ?*x* = (instance-address a))
(make-instance a of A)
(class ?*x*)
(clear)
(defclass A (is-a USER) (role concrete))
(make-instance a of A)
(clear)
(initialize-instance 34)
(clear)
(defclass A (is-a USER) (role concrete)
  (multislot foo (create-accessor write)))
(make-instance a of A)
(mv-slot-insert a foo 1 (instances))
(clear)
(defclass A (is-a USER) (role concrete)
  (slot foo (create-accessor write)))
(make-instance a of A)
(mv-slot-insert a foo 1 abc def)
(clear)
(ppdefmessage-handler USER foo around)
(clear)
(defclass A (is-a USER) (role concrete))
(make-instance a of A)
(defmessage-handler A foo ()
  (+ ?a 2)
  (bind ?a 1))
(send [a] foo)
(clear)
(defclass A (is-a USER) (role concrete)
  (slot foo (default ?NONE) (create-accessor write)))
(make-instance a of A)
(clear)
(defclass A (is-a USER) (role concrete)
  (slot foo (default 100)
            (access read-only)))
(make-instance a of A)
(send [a] put-foo)
(clear)
(ppinstance)
(clear)
(defmessage-handler INTEGER print ()
  (ppinstance))
(send 34 print)
(clear)
(call-next-handler)
(clear)
(defclass A (is-a USER) (role concrete))
(make-instance a of A)
(send [a] bogus-message)
(clear)
(defclass A (is-a USER) (role concrete))
(defmessage-handler USER foo (?a ?b))
(make-instance a of A)
(send [a] foo)
(clear)
(make-instance 34 of A)
(clear)
(defclass A (is-a USER) (role abstract))
(make-instance 34 of A)
(clear)
(defclass A (is-a USER) (role concrete))
(make-instance 34 of A)
(clear)
(make-instance a of 34)
(clear)
(defclass A (is-a USER) (role concrete))
(make-instance a of A (34 override-value))
(clear)
(make-instance [foo] of USER)
(clear)
(defclass A (is-a USER) (role concrete)
  (slot foo (create-accessor write)))
(make-instance a of A (foo 1 2 3 4))
(clear)
(defclass A (is-a USER) (role concrete))
(defmessage-handler A delete around ()
  (if (neq (instance-name ?self) [a]) then
    (call-next-handler)))
(make-instance a of A)
(make-instance a of A)
(undefmessage-handler A delete around)
(clear)
(defclass A (is-a USER) (role concrete))
(make-instance a of A)
(defmessage-handler A init after ()
   (initialize-instance ?self))
(initialize-instance a)
(clear)
(defclass A (is-a USER) (role concrete)
  (slot foo (create-accessor write)))
(defmessage-handler A put-foo after ($?any)
  (delete-instance))
(make-instance a of A (foo 2))
(clear)
(defclass A (is-a USER) (role concrete)
  (slot foo (create-accessor write)))
(make-instance a of A (foo (make-instance a of A)))
(clear)
(defclass A (is-a USER))
(defrule no-class (object (is-a BOGUS)) =>)
