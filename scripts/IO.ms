clearListener()
IsBigEndian = false

/** UTILITY **/
fn SetBigEndian = (
	IsBigEndian = true
)

fn SetLittleEndian = (
	IsBigEndian = false
)

/** SWAP FUNCTIONS **/
fn SwapLongLong v = ( -- Reverse Endianness of a long long
	v = bit.swapBytes v 1 8
	v = bit.swapBytes v 2 7
	v = bit.swapBytes v 3 6
	v = bit.swapBytes v 4 5
	return v
)

fn SwapLong v = ( -- Reverse Endianness of a long
	v = bit.swapBytes v 1 4
	v = bit.swapBytes v 2 3
	return v
)

fn SwapShort v = ( -- Reverse Endianness of a short
	v = bit.swapBytes v 1 2
	return v
)

fn SwapFloat v = ( -- Reverse Endianness of a float
	-- Weird syntax is necessary because bit.swapBytes converts the value to an integer
	-- From the Sonic Generations .model importer
	i = bit.floatAsInt v
	h = bit.intashex i
	while h.count < 8 do h = "0" + h
	
	s = (substring h 7 2) + (substring h 5 2) + (substring h 3 2) + (substring h 1 2)
	v = bit.intAsFloat (bit.hexasint s)
	return v
)

/** READ FUNCTIONS **/
-- Read Long Long
fn ReadLongLongAgnostic fstream asSigned = ( -- Read a long long (Endian-agnostic)
	v = ReadLongLong fstream asSigned
	if (IsBigEndian == true) then v = SwapLongLong v
	return v
)

fn ReadLongLongBE fstream asSigned = ( -- Read a long long (force Big Endian)
	v = ReadLongLong fstream asSigned
	v = SwapLongLong v
	return v
)

fn ReadLongLongLE fstream asSigned = ( -- Read a long (force Little Endian)
	v = ReadLongLong fstream asSigned
	return v
)

-- Read Long
fn ReadLongAgnostic fstream asSigned = ( -- Read a long (Endian-agnostic)
	v = ReadLong fstream asSigned
	if (IsBigEndian == true) then v = SwapLong v
	return v
)

fn ReadLongBE fstream asSigned = ( -- Read a long (force Big Endian)
	v = ReadLong fstream asSigned
	v = SwapLong v
	return v
)

fn ReadLongLE fstream asSigned = ( -- Read a long (force Little Endian)
	v = ReadLong fstream asSigned
	return v
)

-- Read Short
fn ReadShortAgnostic fstream asSigned = ( -- Read a short (Endian-agnostic)
	v = ReadShort fstream asSigned
	if (IsBigEndian == true) then v = SwapShort v
	return v
)

fn ReadShortBE fstream asSigned = ( -- Read a short (force Big Endian)
	v = ReadShort fstream asSigned
	v = SwapShort v
	return v
)

fn ReadShortLE fstream asSigned = ( -- Read a short (force Little Endian)
	v = ReadShort fstream asSigned
	return v
)

-- Read Float
fn ReadFloatAgnostic fstream = ( -- Read a float (Endian-agnostic)
	v = ReadFloat fstream
	if (IsBigEndian == true) then v = SwapFloat v
	return v
)

fn ReadFloatBE fstream = ( -- Read a float (force Big Endian)
	v = ReadFloat fstream
	v = SwapFloat v
	return v
)

fn ReadFloatLE fstream = ( -- Read a float (force Little Endian)
	v = ReadFloat fstream
	return v
)

/** WRITE FUNCTIONS **/
-- Write Long Long
fn WriteLongLongAgnostic fstream v asSigned = ( -- Write a long long (Endian-agnostic)
	if (IsBigEndian == true) then v = SwapLongLong v
	WriteLongLong fstream v asSigned
)

fn WriteLongLongBE fstream v asSigned = ( -- Write a long long (force Big Endian)
	v = SwapLongLong v
	WriteLongLong fstream v asSigned
)

fn WriteLongLongLE fstream v asSigned = ( -- Write a long long (force Little Endian)
	WriteLongLong fstream v asSigned
)

-- Write Long
fn WriteLongAgnostic fstream v asSigned = ( -- Write a long (Endian-agnostic)
	if (IsBigEndian == true) then v = SwapLong v
	WriteLong fstream v asSigned
)

fn WriteLongBE fstream v asSigned = ( -- Write a long (force Big Endian)
	v = SwapLong v
	WriteLong fstream v asSigned
)

fn WriteLongLE fstream v asSigned = ( -- Write a long (force Little Endian)
	WriteLong fstream v asSigned
)

-- Write Short
fn WriteShortAgnostic fstream v asSigned = ( -- Write a short (Endian-agnostic)
	if (IsBigEndian == true) then v = SwapShort v
	WriteShort fstream v asSigned
)

fn WriteShortBE fstream v asSigned = ( -- Write a short (force Big Endian)
	v = SwapShort v
	WriteShort fstream v asSigned
)

fn WriteShortLE fstream v asSigned = ( -- Write a short (force Little Endian)
	WriteShort fstream v asSigned
)

-- Write Float
fn WriteFloatAgnostic fstream v = ( -- Write a float (Endian-agnostic)
	if (IsBigEndian == true) then v = SwapFloat v
	WriteFloat fstream v
)

fn WriteFloatBE fstream v = ( -- Write a float (force Big Endian)
	v = SwapFloat v
	WriteFloat fstream v
)

fn WriteFloatLE fstream v = ( -- Write a float (force Little Endian)
	WriteFloat fstream v
)