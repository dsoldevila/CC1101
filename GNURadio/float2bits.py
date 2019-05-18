#from rflib import *
import sys, bitstring
import numpy as np
 
"""
1010 1010 1010 1010 1010 1010 1010 1010 PREAMBLE
0101 0111 0100 0011 SYNC WORD (x2)
0000 0111 0000 0111 0000 0111 0000 0111 0000 0111 0000 0111 DATA (0x07 x7)
""" 
def get_bitstring(symbols):
	bs = ''
	for s in symbols:
    		if s > 0.6:
        		bs += '1'
    		else:
        		bs += '0'
	print(bs)
 
def bits2bytes(bit_string):
	return bitstring.BitArray(bin=str(bit_string.strip())).tobytes()
 
def xmit(bit_string):
	try:
		d.RFxmit((bits2bytes(bit_string)) * 10)
	except ChipconUsbTimeoutException:
		pass
	except:
		print("Invalid bit string or xmit error")
		sys.exit()

if __name__=="__main__":
	with open(sys.argv[1], 'rb') as infile:
		line="a"
		data = np.fromfile(infile, np.float32)
	
		data = list(data)
		get_bitstring(data)
		#print(line)
	
