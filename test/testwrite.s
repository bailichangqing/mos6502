
.export _testwrite

.import _write
.import __CARTROM_START__
.importzp sp

.segment "RODATA"


hello:
	.byte $48, $45, $4C, $4C, $4F, $0A, $00

.segment "CODE"


_testwrite:
	ldy #0
	lda #<(hello)
	ldx #>(hello)
	sta (sp), y
	iny
	txa
	sta (sp), y
	iny
	lda #$1 ; stdout
	ldx #$0 ; 
	sta (sp), y
	iny
	txa
	sta (sp), y
	iny
	lda #$7
	ldx #$0
	jsr _write
	rts
	
	

