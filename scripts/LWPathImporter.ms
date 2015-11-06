clearListener()
fileIn "IO.ms"

global yzflip = true
global MeterScale = 10
	
fn GetString fstream offset = (
	cur = ftell fstream
	fseek fstream offset #seek_set
	str = ReadString fstream
	fseek fstream cur #seek_set
	return str
)

path_file = getOpenFileName caption:"Load paths" types:"Sonic Lost World Path (*.path2.bin)|*.path2.bin"

if (path_file != undefined) do (
	f = fopen path_file "rb"
	
	magic = ReadLongLE f #unsigned
	if (magic == 0x48544150) then SetBigEndian()
	else SetLittleEndian()
	
	fseek f 0x4 #seek_cur
	NumPaths = ReadLongAgnostic f #unsigned
	format "Num paths %\n" NumPaths
	fseek f 0x4 #seek_cur

	for p = 1 to NumPaths do (
		PathNameStr = GetString f (ReadLongAgnostic f #unsigned)
		format "%. %\n" p PathNameStr
		fseek f 0x2 #seek_cur
		KnotCount = ReadShortAgnostic f #unsigned
		fseek f 0xC #seek_cur
		KnotsOffset = ReadLongAgnostic f #unsigned
		fseek f 0x8 #seek_cur
		DoubleKnotCount = ReadLongAgnostic f #unsigned
		DoubleKnotsOffset = ReadLongAgnostic f #unsigned
		fseek f 0x28 #seek_cur
		end = ftell f
		
		if (DoubleKnotCount > 0) then (
			fseek f DoubleKnotsOffset #seek_set
			
			spline = line name:(PathNameStr)
			addnewspline spline
			addnewspline spline
			
			for v = 1 to DoubleKnotCount do (
				-- Double spline knot data has the two splines interleaved
				-- spline1knot1, spline2knot1, spline1knot2, etc
				local knot
				
				if (yzflip == true) then (
					x = ReadFloatAgnostic f
					z = ReadFloatAgnostic f
					y = -(ReadFloatAgnostic f)
					knot = [x, y, z]
				)
				else (
					x = ReadFloatAgnostic f
					y = ReadFloatAgnostic f
					z = ReadFloatAgnostic f
					knot = [x, y, z]
				)
				
				knot /= MeterScale
				addknot spline ((mod (v-1) 2) + 1) #corner #line knot 
			)
		)
		
		else (
			fseek f KnotsOffset #seek_set
			spline = line name:PathNameStr
			addnewspline spline
			
			for v = 1 to KnotCount do (
				local knot
				
				if (yzflip == true) then (
					x = ReadFloatAgnostic f
					z = ReadFloatAgnostic f
					y = -(ReadFloatAgnostic f)
					knot = [x, y, z] / MeterScale
				)
				else (
					x = ReadFloatAgnostic f
					y = ReadFloatAgnostic f
					z = ReadFloatAgnostic f
					knot = [x, y, z] / MeterScale
				)
				addknot spline 1 #corner #line knot
			)
			updateshape spline
		)
		
		fseek f end #seek_set
	)
	
	fclose f
)