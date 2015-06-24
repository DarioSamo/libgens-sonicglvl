-- Functions taken/modified from the Sonic Generations .model importer
clearListener()

fn floatSwap2 f = ( -- Reverses byte order on floats
	i = bit.floatAsInt f
	h = bit.intashex i
	while h.count < 8 do h = "0" + h
	
	s = (substring h 7 2) + (substring h 5 2) + (substring h 3 2) + (substring h 1 2)
	bit.intAsFloat (bit.hexasint s)
)	

fn ReadBELong fstream = ( -- Reeads Big Endian 32-bit integers
	long = readlong fstream
	long = bit.swapBytes long 1 4
	long = bit.swapBytes long 2 3
	if long != undefined then return long
	else return 0
)

fn ReadBELongLong fstream = ( -- Reads Big Endian 64-bit integers
	longlong = readlonglong fstream
	longlong = bit.swapBytes longlong 1 8
	longlong = bit.swapBytes longlong 2 7
	longlong = bit.swapBytes longlong 3 6
	longlong = bit.swapBytes longlong 4 5
	if longlong != undefined then return longlong
	else return 0
)

fn ReadUBELong fstream = ( -- Reads Big Endian 32-bit integers
	long = readlong fstream #unsigned
	long = bit.swapBytes long 1 4
	long = bit.swapBytes long 2 3
	if long != undefined then return long
	else return 0
)

fn ReadBEShort fstream = ( -- Reads Big Endian 16-bit shorts
	short = readshort fstream
	short = bit.swapBytes short 1 2
	if short != undefined then return short
	else return 0
)

fn ReadSBEShort fstream = ( -- Reads Big Endian 16-bit signed shorts
	short = readshort fstream #signed
	short = bit.swapBytes short 1 2
	if short != undefined then return short
	else return 0
)
 
fn ReadUBEShort fstream = ( -- Reads Big Endian 16-bit unsigned shorts
	short = readshort fstream #unsigned
	short = bit.swapBytes short 1 2
	if short != undefined then return short
	else return 0
)

fn ReadBEFloat fstream = ( -- Reads Big Endian 32-bit floats
	return floatSwap2(readfloat fstream)
)

fn WriteBELong fstream long = ( -- Writes Big Endian 32-bit integers
	long = bit.swapBytes long 1 4
	long = bit.swapBytes long 2 3
	WriteLong fstream long
)

fn WriteUBELong fstream long = ( -- Writes Big Endian 32-bit integers
	long = bit.swapBytes long 1 4
	long = bit.swapBytes long 2 3
	WriteLong fstream long #unsigned
)

fn WriteSBELong fstream long = ( -- Writes Big Endian 32-bit integers
	long = bit.swapBytes long 1 4
	long = bit.swapBytes long 2 3
	WriteLong fstream long #signed
)

fn WriteBEShort fstream short = ( -- Writes Big Endian 16-bit shorts
	short = bit.swapBytes short 1 2
	WriteShort fstream short
)

fn WriteUBEShort fstream short = ( -- Writes Big Endian 16-bit unsigned shorts
	short = bit.swapBytes short 1 2
	WriteShort fstream short #unsigned
)

fn WriteBEFloat f floatval = ( -- Writes Big Endian 32-bit floats
	h = bit.intashex (bit.floatAsInt floatval)
	while h.count < 8 do h = "0" + h
	s = (substring h 7 2) + (substring h 5 2) + (substring h 3 2) + (substring h 1 2)
	WriteFloat f (bit.intAsFloat (bit.hexasint s))
)