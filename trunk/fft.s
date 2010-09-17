	.global _vectecFIR
_vectecFIR:
	push w4
	push w5
	push w8
	push w10
	push w15

	;Save old circular buffer
	push MODCON
	push XMODSRT
	push XMODEND
	push YMODSRT
	push YMODEND

	push CORCON

	;Unsigned Integer
	mov #0x10D1, w10
	mov w10, CORCON
	;Activate the circular buffer on w8
	mov	#0x8F08,w10
	mov	w10,MODCON
	
	mov w0, XMODSRT
	;(2 * FFT_BLOCK_LENGTH) - 1
	add #127, w0
	mov w0, XMODEND
	nop

	;Correct starting address for respective sample number and insert the latest
	mov XMODSRT, w8
	;w2 is counts of 16 bit width, w8 is counts of 8 bit width, so we add w2 twice to w8 (multiply by two and add to w8)
	add w2, w8, w8
	add w2, w8, w8
	mov w1, w10

	clr A, [w8]-=2, w4, [w10]+=2, w5
	;FFT_BLOCK_LENGTH - 1
	repeat #63
	mac w4*w5, A, [w8]-=2, w4, [w10]+=2, w5

	;LOG2_BLOCK_LEGNTH - 4
	sac A, #2, w0

	pop CORCON

	pop YMODEND
	pop YMODSRT
	pop XMODEND
	pop XMODSRT
	pop MODCON

	pop w15
	pop w10
	pop w8
	pop w5
	pop w4
	return
	.end
