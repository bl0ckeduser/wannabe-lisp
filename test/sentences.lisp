;; meme sentence generator
;; (C) 2015 electric certified clown laboratories inc. (PhD) (MSc.)
;; (C) 2015 (C) (TM) (R) (C) 2001 (C) (TM)
;; to be fair PAIP by peter norvig has a very similar example
;; with "the man hit the table" etc.

(define sentences
	(list
		(list	
			'(the (? x) was (? y))
			(list	
				'(doctor '(an analyst))
				'(everything ephemeral)
				'(attachment deleted)
				'(proof '(in the pudding))
				'(president '(an analyst))
				'(clown '(an athlete))
				'(cake '(a lie))
			)
		)

		(list
			'(that's (? x) with (? y))
			(list
				'(business '(dot net))
				'(cooking cookbook)
				'(trolling panache)
				'(repetition parrot)
			)
		)
	)
)

(define (flatten u)
	(filter 
		(lambda (u) 
			(not (eq? u 'quote))	
		)
		(apply append (apply append u))
	)
)

(map
	(lambda 
		(sentence)
		(begin
			(let (
				(skel (car sentence))
				(subs (cadr sentence))	
			     )
			(display '---skel:)
			(display skel)
			(newline)

			(map
				(lambda (sub)
					(begin
						(define (makesub skp sbp)
							
							(cond
							  
							     ( (null? skp)
								'NIL
							     )

							     ( (null? sbp)
								'NIL
							     )

							    ((pair? (car skp))
								;; substitute
								(cons
									(car sbp)
									(makesub (cdr skp) (cdr sbp))
								))


							     (else
								;; dont substitute
								(cons 
									(car skp)
									(makesub (cdr skp) sbp)
								)
							     )
							)
						)
						(display (flatten (makesub skel sub)))
						(newline)
					)
				)

				subs
			)

			(display '============================)
			(newline)
			)
		)
	)
	sentences
)


