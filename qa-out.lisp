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
