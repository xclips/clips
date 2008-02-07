(unwatch all)
(clear)
(set-strategy depth)
(open "Results//chksyntx.rsl" chksyntx "w")
(dribble-on "Actual//chksyntx.out")
(batch "chksyntx.bat")
(dribble-off)
(load "compline.clp")
(printout chksyntx "chksyntx.bat differences are as follows:" crlf)
(compare-files "Expected//chksyntx.out" "Actual//chksyntx.out" chksyntx)
(close chksyntx)