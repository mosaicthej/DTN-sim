; Measuring TCP roundtrip time with SRTT (using Karn's algorithm)

; SRTT_i = (\alpha * SRTT_{i-1}) + ((1 - \alpha) * S_{i}) 
; where:
;   SRTT_i: Smoothed Round-Trip Time at time i
;   S_i   : Sample RTT at time i
;   \alpha: Smoothing factor, 0 < \alpha < 1
;   SRTT_0: Initial value of SRTT

; and, to determine the retransmission timeout (RTO) value:
;  RTO_i = \beta * SRTT_i, use beta > 1

; (srtt  srtt_{i-1} sample \alpha)
; returns the new SRTT value
(defun srtt_next (srtt sample alpha)
  (+ (* alpha srtt) (* (- 1 alpha) sample)))

; foldmap: (func init Xs) -> Ys. Where:
; func: (Y X) -> Y; init: Y; Xs: [X]; Ys: [Y]
(defun foldmap (func init Xs)
  (if (null Xs)
    nil  ; (list init)
    (let ((Y (apply func (list init (car Xs)))))
      (cons Y (foldmap func Y (cdr Xs))))))

; then, redefine srttListFold using foldmap and 
; a partial application of srtt_next
(defun srttListFold (fsrtt srtt0 Ss alpha) 
  (foldmap 
    (lambda ; partial apply alpha
      (srtt0 sample) ; now this is a function takes (Y X)->Y.
      (apply fsrtt (list srtt0 sample alpha)))
    srtt0 Ss))

; a-1
; A TCP sneder's SRTT is 180 ms, 
; but then a routing change occurs,
; fter which all measured RTTs are 110ms.
; How many measurements of the new RTT are required 
; before SRTT drops below 130ms?

(defun srtt_uniform_target (srttNextf init unisample alpha target)
  (labels ((srtt_uniform_target_inner (srttNow target comparator iter)
    (if (funcall comparator srttNow target) iter
      (srtt_uniform_target_inner
        (funcall srttNextf srttNow unisample alpha) 
        target comparator (+ 1 iter)))))
  (srtt_uniform_target_inner init target #'< 0)))

; rttvar <- (1 - \beta) * RTTVAR + \beta * |SRTT - R'| 
(defun rttvar_next (rttvar srtt sample beta)
  (+ (* (- 1 beta) rttvar)
     (* beta (abs (- sample srtt)))))

; after srtt and rttvar is calculated, 
; RTO <- SRTT + 4 * RTTVAR
(defun rto_next (srtt rttvar)
  (+ srtt (* 4 rttvar)))

; get the triplets of (srtt, rttvar, rto) 
; from next sample
(defun rtoTriplet (srtt rttvar sample alpha beta)
  (let ((rttvarNext (rttvar_next rttvar srtt sample beta))
        (srttNext (srtt_next srtt sample alpha)))
    (let ((rtoNext (rto_next srttNext rttvarNext)))
      (list srttNext rttvarNext rtoNext))))


