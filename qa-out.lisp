(load "qa-tcp.lisp")
(let ((sample_c (srtt_uniform_target #'srtt_next 180 110 0.8 130)))
  (let ((srttList 
          (srttListFold 'srtt_next 180 
            (loop repeat sample_c collect 110) 0.8)))
    (format t "~d" "a-1-1:")(terpri)
    (format t "~d" "need ")(write sample_c)(format t "~d" " samples")
    (format t "~d" " to drop below 130ms")(terpri)
    (format t "~d" "srtt for first ")(write sample_c)
    (format t "~d" " samples:")(terpri)
    (write srttList)(terpri)))

(let ((sample_c (q2_query 10000 600)))
  (terpri) (terpri)
  (format t "~d" "a-1-2:") (terpri)
  (format t "~d" "need ") (write sample_c) (format t "~d" " samples") (terpri)
  (format t "~d" " so next 600ms RTT will trigger a timeout") (terpri)
  (format t "~d" " the first ")(write sample_c)(format t "~d" " samples," )
  (format t "~d" " (srtt, rttvar, rto) are")

  (write (loop for i from 1 to sample_c collect
           (rto_uniform_ntimes #'srtt_next #'rttvar_next #'rto_next 
                                200 600 i 0.8 0.7 10000)))
  (terpri)(terpri)
  (format t "~d" "the first 60 samples in the smallest exceeding case are:")
  (terpri)
  (write (rtoTripletListFold #'srtt_next #'rttvar_next 0 0
    (loop for i from 1 to 60 collect (if (= (mod i (+ sample_c 1)) 0) 200 600))
    0.8 0.7)))
