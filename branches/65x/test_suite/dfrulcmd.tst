(unwatch all)
(clear)
(dribble-on "Actual//dfrulcmd.out")
(batch "dfrulcmd.bat")
(dribble-off)
(clear)
(open "Results//dfrulcmd.rsl" dfrulcmd "w")
(load "compline.clp")
(printout dfrulcmd "dfrulcmd.bat differences are as follows:" crlf)
(compare-files "Expected//dfrulcmd.out" "Actual//dfrulcmd.out" dfrulcmd)
(close dfrulcmd)