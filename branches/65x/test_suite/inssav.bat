(reset)
(set-current-module MAIN)
(generate-instances BAR 100 B D E)
(generate-instances WOZ 100 F G)
(instances *)
(instances)
(set-current-module BAR)
(instances)
(bsave-instances "Temp//inssav1.bin")
(bload-instances inssav.clp)
(save-instances "Actual//inssav1.out")

(progn
  (bind ?start (time))
  (restore-instances "Actual//inssav1.out")
  (bind ?tt (- (time) ?start))
  (bind ?start (time))
  (bload-instances "Temp//inssav1.bin")
  (bind ?bt (- (time) ?start))
  (if (> ?bt ?tt)
     then
     (printout t "Binary load time exceeded text load time" crlf)))
(save-instances "Actual//inssav1.out")
(file-message "Actual//inssav1.out" "D E")
(save-instances "Actual//inssav2.out" gobbledy-gook)
(bsave-instances "Temp//inssav2.bin" gobbledy-gook)
(save-instances "Actual//inssav2.out" 123)
(bsave-instances "Temp//inssav2.bin" 123)
(bsave-instances "Temp//inssav2.bin" local)
(bload-instances "Temp//inssav2.bin")
(save-instances "Actual//inssav2.out" local)
(file-message "Actual//inssav2.out" "D E")
(bsave-instances "Temp//inssav3.bin" visible)
(bload-instances "Temp//inssav3.bin")
(save-instances "Actual//inssav3.out" visible)
(file-message "Actual//inssav3.out" "INITIAL-OBJECT B D E")
(save-instances "Actual//inssav4.out" local inherit)
(save-instances "Actual//inssav4.out" local inherit A)
(save-instances "Actual//inssav4.out" local A)
(bsave-instances "Temp//inssav4.bin" visible inherit A)
(bload-instances "Temp//inssav4.bin")
(save-instances "Actual//inssav4.out" visible inherit A)
(file-message "Actual//inssav4.out" "B E")
(save-instances "Actual//inssav5.out" local C)
(save-instances "Actual//inssav5.out" local inherit inherit)
(save-instances "Actual//inssav5.out" local inherit C)
(file-message "Actual//inssav5.out" "D E")
(set-current-module FOO)
(instances)
(open "Actual//inssav6.out" inssav6 "w")
(close inssav6)
(save-instances "Actual//inssav6.out" local inherit B)
(file-message "Actual//inssav6.out" "no classes")
(save-instances "Actual//inssav7.out" visible inherit B)
(file-message "Actual//inssav7.out" "B E F")