	.global _sample
_sample:
	;w2 in 16 bit counts, w0 & w1 in 8 bit counts... Need 2 w2s for each w0 & w1
	add w2, w0, w0
	add w2, w0, w0
	add w2, w1, w1
	add w2, w1, w1
	mov w3, [w0]
	mov w3, [w1]
	return
	.end
