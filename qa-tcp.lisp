; Measuring TCP roundtrip time with SRTT (using Karn's algorithm)

; foldmap: (func init Xs) -> Ys. Where:
; func: (Y X) -> Y; init: Y; Xs: [X]; Ys: [Y]
(defun foldmap (func init Xs)
  (if (null Xs) nil  ; (list init)
    (let ((Y (apply func (list init (car Xs)))))
      (cons Y (foldmap func Y (cdr Xs))))))

; foldmap2: (funcY funcZ initZ Xs) -> ((Y, Z)s). Where:
; funcY: (Y X) -> Y; funcZ: (Z Y X) -> Z; initZ: Z; Xs: [X]; Ys: [Y]
(defun foldmap2 (funcXY funcXZ initY initZ Xs)
  (if (null Xs) nil
    (let ((Yn (apply funcXY (list initY (car Xs))))
          (Zn (apply funcXZ (list initZ initY (car Xs)))))
      (cons (list Yn Zn) (foldmap2 funcXY funcXZ Yn Zn (cdr Xs))))))

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

; use foldmap2 since need to update both srtt and rttvar
; (foldmap2 funcXY funcXZ initY initZ Xs) 
; fsrtt: (srtt sample alpha) -> srttNext
; frttvar: (rttvar srtt sample beta) -> rttvarNext
(defun rttvarListFold (fsrtt frttvar srtt0 rttvar0 Ss alpha beta)
  (foldmap2
    ; funcXY: sample -> srttNext
    (lambda (srtti R) (funcall fsrtt srtti R alpha))
    ; funcXZ: sample -> rttvarNext
    (lambda (rttvari srtti R) (funcall frttvar rttvari srtti R beta))
    ; initY: new srtt; initZ: new rttvar
    (funcall fsrtt srtt0 (car Ss) alpha)
    (funcall frttvar rttvar0 srtt0 (car Ss) beta)
    ; Xs: samples
    (cdr Ss)))

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


