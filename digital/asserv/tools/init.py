"""Default parameters for asserv."""
host = dict (
	scale = 0.0395840674352314, f = 0xdd1,
	tkp = 1, tkd = 16, akp = 2, akd = 16,
	a0p = 1, a0d = 16,
	E = 0x3ff, D = 0x1ff,
	ta = 0.5, aa = 0.5, tsm = 0x60, tss = 0x40, asm = 0x60, ass = 0x40,
	a0a = 0.5, a0sm = 0x10, a0ss = 0x05,
	)
target = dict (
	scale = 0.0413530725332892, f = 0xcfa,
	c = float (0xffefbe) / (1 << 24),
	tkp = 1, tkd = 16, akp = 2, akd = 16,
	a0p = 0.8, a0i = 0.05, a0d = 0.05,
	E = 0x3ff, D = 0x1ff, I = 0x1fff,
	ta = 0.5, aa = 0.5, tsm = 0x60, tss = 0x40, asm = 0x60, ass = 0x40,
	a0a = 0.5, a0sm = 0x10, a0ss = 0x05,
	)
